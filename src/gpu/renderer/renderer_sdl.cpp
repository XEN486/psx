#include "renderer_sdl.hpp"
#include "../gpu.hpp"

#include <vector>
#include <print>
#include <optional>

using namespace Gpu;

int8_t dither_table[4][4] = {
	{ -4, +0, -3, +1 },
	{ +2, -2, +3, -1 },
	{ -3, +1, -4, +0 },
	{ +3, -1, +2, -2 }
};

static inline uint8_t saturating_add(uint8_t a, int8_t b) {
	int16_t r = a + b;
	if (r > 255) return 255;
	if (r < 0) return 0;
	return (uint8_t)r;
}

static inline Color dither_color(Position position, Color color) {
	int8_t offset = dither_table[position.y & 3][position.x & 3];
	color.r = saturating_add(color.r, offset);
	color.g = saturating_add(color.g, offset);
	color.b = saturating_add(color.b, offset);
	
	return color;
}

void RendererSDL::DrawPixel(Position position, Color color) {
	size_t index = position.y * 1024 + position.x;
	uint16_t value = m_VRAM[index];

	if ((value & 0x8000) && m_Params->preserve_masked_pixels) return;
	if (m_Params->force_set_mask_bit) color.b15 = true;

	m_VRAM[index] = color.Get555();
}

static inline int32_t edge_function(Position a, Position b, Position p) {
	return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void RendererSDL::DrawTriangle(DrawPolygonOptions& options, std::span<Position> positions, std::span<Color> colors, std::span<UV> uvs) {
	for (auto& p : positions) {
		p.x += m_Params->drawing_x_offset;
		p.y += m_Params->drawing_y_offset;
	}

	// check bounding box
	int16_t min_x = std::min(positions[0].x, std::min(positions[1].x, positions[2].x));
	int16_t min_y = std::min(positions[0].y, std::min(positions[1].y, positions[2].y));
	int16_t max_x = std::max(positions[0].x, std::max(positions[1].x, positions[2].x));
	int16_t max_y = std::max(positions[0].y, std::max(positions[1].y, positions[2].y));

	min_x = std::max(min_x, (int16_t)m_Params->drawing_area_left);
	max_x = std::min(max_x, (int16_t)m_Params->drawing_area_right);
	min_y = std::max(min_y, (int16_t)m_Params->drawing_area_top);
	max_y = std::min(max_y, (int16_t)m_Params->drawing_area_bottom);

	// return if the bounding box is empty
	if ((min_x > max_x) || (min_y > max_y)) {
		return;
	}

	int32_t area = edge_function(positions[0], positions[1], positions[2]);
	bool clockwise = area < 0;
	
	for (int16_t y = min_y; y <= max_y; y++) {
		for (int16_t x = min_x; x <= max_x; x++) {
			Position p(x,y);
			int32_t w0 = edge_function(positions[1], positions[2], p);
			int32_t w1 = edge_function(positions[2], positions[0], p);
			int32_t w2 = edge_function(positions[0], positions[1], p);

			bool inside = false;
			if (clockwise) {
				inside = w0 <= 0 && w1 <= 0 && w2 <= 0;
			} else {
				inside = w0 >= 0 && w1 >= 0 && w2 >= 0;
			}

			if (inside) {
				// by default use first color for pixel
				Color texel(0, 0, 0);
				Color color = colors[0];

				// make sure we arent going to overwrite a masked pixel
				if (m_Params->preserve_masked_pixels && (m_VRAM[p.y * 1024 + p.x] & 0x8000)) {
					continue;
				}

				// if texturing is enabled then calculate the uv for this pixel and get the texel
				if (options.texture == TextureType::Textured) {
					// interpolate u and v across texture
					int u = (w0 * uvs[0].u + w1 * uvs[1].u + w2 * uvs[2].u) / area;
					int v = (w0 * uvs[0].v + w1 * uvs[1].v + w2 * uvs[2].v) / area;
					
					// apply texture window
					u = (u & ~(m_Params->texture_window_x_mask * 8)) | ((m_Params->texture_window_x_offset & m_Params->texture_window_x_mask) * 8);
					v = (v & ~(m_Params->texture_window_y_mask * 8)) | ((m_Params->texture_window_y_offset & m_Params->texture_window_y_mask) * 8);

					if (m_Params->texture_depth == TextureDepth::T4) {
						uint16_t clut = uvs[0].extra;		// CLUT is stored in first UV
						uint16_t texpage = uvs[1].extra;	// texture page is stored in second UV

						uint16_t texpage_x = (texpage & 0b1111) * 64;
						uint16_t texpage_y = ((texpage >> 4) & 1) * 256;

						uint16_t clut_x = (clut & 0x3f) * 16;
						uint16_t clut_y = (clut >> 6) & 0x1ff;

						// get entry in CLUT
						uint16_t txl = m_VRAM[(texpage_y + v) * 1024 + (texpage_x + (u / 4))];
						int index = (txl >> (u % 4) * 4) & 0xf;
						uint16_t clut_entry = m_VRAM[clut_y * 1024 + clut_x + index];

						// transparent pixel
						if (clut_entry == 0) {
							continue;
						}

						texel = Color(clut_entry);
					}

					else if (m_Params->texture_depth == TextureDepth::T8) {
						uint16_t clut = uvs[0].extra;		// CLUT is stored in first UV
						uint16_t texpage = uvs[1].extra;	// texture page is stored in second UV

						uint16_t texpage_x = (texpage & 0b1111) * 64;
						uint16_t texpage_y = ((texpage >> 4) & 1) * 256;

						uint16_t clut_x = (clut & 0x3f) * 16;
						uint16_t clut_y = (clut >> 6) & 0x1ff;

						// get entry in CLUT
						uint16_t txl = m_VRAM[(texpage_y + v) * 1024 + (texpage_x + (u / 2))];
						int index = (txl >> (u % 2) * 8) & 0xff;
						uint16_t clut_entry = m_VRAM[clut_y * 1024 + clut_x + index];

						// transparent pixel
						if (clut_entry == 0) {
							continue;
						}

						texel = Color(clut_entry);
					}

					else if (m_Params->texture_depth == TextureDepth::T15) {
						uint16_t texpage = uvs[1].extra; // texture page is stored in second UV

						uint16_t texpage_x = (texpage & 0b1111) * 64;
						uint16_t texpage_y = ((texpage >> 4) & 1) * 256;

						uint16_t value = m_VRAM[(texpage_y + v) * 1024 + (texpage_x + u)];
						if (value == 0) {
							continue;
						}

						texel = Color(value);
					}
				}
				
				// if gouraud shading is enabled, calculate the color by interpolating it
				if (options.shading == ShadingType::Gouraud) {
					color.r = uint8_t((w0*colors[0].r + w1*colors[1].r + w2*colors[2].r) / area);
					color.g = uint8_t((w0*colors[0].g + w1*colors[1].g + w2*colors[2].g) / area);
					color.b = uint8_t((w0*colors[0].b + w1*colors[1].b + w2*colors[2].b) / area);
				}

				if (options.texture == TextureType::Textured) {
					if (options.modulation == ModulationType::Modulated) {
						color.r = uint8_t(float(texel.r * color.r) / 128.f);
						color.g = uint8_t(float(texel.g * color.g) / 128.f);
						color.b = uint8_t(float(texel.b * color.b) / 128.f);
					} else {
						color = texel;
					}
				}

				// if dithering is enabled and gouraud/modulation is selected then dither
				bool modulated = options.texture == TextureType::Textured && options.modulation == ModulationType::Modulated;
				if (m_Params->dithering && (options.shading == ShadingType::Gouraud || modulated)) {
					color = dither_color(p, color);
				}

				// draw the pixel
				if (options.transparency == TransparencyType::SemiTransparent && (options.texture == TextureType::Untextured || (options.texture == TextureType::Textured && texel.b15))) {
					Color b = Color(m_VRAM[p.y * 1024 + p.x]);
					Color f = color;
					switch (m_Params->semi_transparency) {
						case 0: {
							color.r = (uint8_t)std::min(255, (int)(0.5f * b.r) + (int)(0.5f * f.r));
							color.g = (uint8_t)std::min(255, (int)(0.5f * b.g) + (int)(0.5f * f.g));
							color.b = (uint8_t)std::min(255, (int)(0.5f * b.b) + (int)(0.5f * f.b));
							break;
						}

						case 1: {
							color.r = (uint8_t)std::min(255, (int)b.r + (int)f.r);
							color.g = (uint8_t)std::min(255, (int)b.g + (int)f.g);
							color.b = (uint8_t)std::min(255, (int)b.b + (int)f.b);
							break;
						}

						case 2: {
							color.r = (uint8_t)std::max(0, (int)b.r - (int)f.r);
							color.g = (uint8_t)std::max(0, (int)b.g - (int)f.g);
							color.b = (uint8_t)std::max(0, (int)b.b - (int)f.b);
							break;
						}

						case 3: {
							color.r = (uint8_t)std::min(255, (int)b.r + (int)(0.25f * f.r));
							color.g = (uint8_t)std::min(255, (int)b.g + (int)(0.25f * f.g));
							color.b = (uint8_t)std::min(255, (int)b.b + (int)(0.25f * f.b));
							break;
						}
					}
				}

				RendererSDL::DrawPixel(p, color);
			}
		}
	}
}

void RendererSDL::DrawPolygon(DrawPolygonOptions& options, std::span<Position> positions, std::span<Color> colors, std::span<UV> uvs) {
	switch (options.polygon) {
		case PolygonType::Triangle: {
			RendererSDL::DrawTriangle(options, positions, colors, uvs);
			break;
		}

		case PolygonType::Quad: {
			// split into 2 triangles: [0, 1, 2] and [1, 2, 3]
			std::vector<Position> t1_vertices = { positions[0], positions[1], positions[2] };
			std::vector<Color> t1_colors = { colors[0], colors[1], colors[2] };
			std::vector<UV> t1_uvs;

			if (options.texture == TextureType::Textured) t1_uvs = { uvs[0], uvs[1], uvs[2] };

			std::vector<Position> t2_vertices = { positions[1], positions[2], positions[3] };
			std::vector<Color> t2_colors = { colors[1], colors[2], colors[3] };
			std::vector<UV> t2_uvs;

			if (options.texture == TextureType::Textured) {
				t2_uvs = { uvs[1], uvs[2], uvs[3] };

				// preserve texpage and clut
				t2_uvs[0].extra = uvs[0].extra;
				t2_uvs[1].extra = uvs[1].extra;
			}

			RendererSDL::DrawTriangle(options, t1_vertices, t1_colors, t1_uvs);
			RendererSDL::DrawTriangle(options, t2_vertices, t2_colors, t2_uvs);

			break;
		}
	}
}

void RendererSDL::Draw() {
	// clear the screen
	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(m_Renderer);

	// lock the texture for writing
	void* pixels;
	int pitch;
	SDL_LockTexture(m_Texture, nullptr, &pixels, &pitch);

	uint32_t* dst = static_cast<uint32_t*>(pixels);
	const uint16_t* src = reinterpret_cast<const uint16_t*>(m_VRAM.data());

	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 1024; ++x) {
			Color color(src[y * 1024 + x]);
			
			// map to RGBA8888
			dst[y * 1024 + x] = (color.r << 24) | (color.g << 16) | (color.b << 8) | 0xff;
		}
	}

	SDL_UnlockTexture(m_Texture);

	int x = m_Params->display_vram_x_start;
	int y = m_Params->display_vram_y_start;
	int width = m_Params->horizontal_resolution.GetPixels(); // 256, 320, 512, 640
	int height = (m_Params->vertical_resolution == VerticalResolution::Y480) ? 480 : 240;

	// render the texture
	SDL_FRect src_rect = {
		(float)x,
		(float)y,
		(float)width,
		(float)height,
	};

	int window_width, window_height;
	SDL_GetWindowSize(m_Window, &window_width, &window_height);

	float scale_x = window_width / (float)width;
	float scale_y = window_height / (float)height;
	float scale = std::min(scale_x, scale_y);

	SDL_FRect dst_rect = {
		(window_width - width * scale) / 2.0f,
		(window_height - height * scale) / 2.0f,
		width * scale,
		height * scale
	};

#ifdef ENABLE_VRAM_VIEW
	SDL_RenderTexture(m_Renderer, m_Texture, NULL, NULL);
#else
	SDL_RenderTexture(m_Renderer, m_Texture, &src_rect, &dst_rect);
#endif
	SDL_RenderPresent(m_Renderer);
}

uint16_t RendererSDL::GetPixel(Position position) {
	return m_VRAM[position.y * 1024 + position.x];
}