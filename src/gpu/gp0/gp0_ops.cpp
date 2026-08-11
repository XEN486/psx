#include "../gpu.hpp"
using namespace Gpu;

void GP0::NOP() {
	// nothing
}

void GP0::DrawModeSetting() {
	auto& params = m_GPU->GetParams();
	u32 word = m_Command[0];

	params.page_base_x = word & 0xf;
	params.page_base_y = (word >> 4) & 1;
	params.semi_transparency = (word >> 5) & 3;
	params.texture_depth = static_cast<TextureDepth>((word >> 7) & 3);
	params.dithering = (word >> 9) & 1;
	params.draw_to_display = (word >> 10) & 1;
	params.texture_disable = (word >> 11) & 1;
	
	params.textured_rectangle_x_flip = (word >> 12) & 1;
	params.textured_rectangle_y_flip = (word >> 13) & 1;
}

void GP0::ClearCache() {
	// not implemented
}

void GP0::ImageLoad() {
	u32 resolution = m_Command[2];
	u16 width = resolution & 0xffff;
	u16 height = (resolution >> 16) & 0xffff;
	uint32_t image_size = width * height;

	// set up GP0 port to receive image data
	m_WordsLeft = (image_size + 1) / 2;
	m_PortState = GP0PortState::ImageLoad;

	// set up image data
	m_ImageLoadOptions = {
		.pos = Position(m_Command[1]),
		.width = width,
		.height = height,
		.current_word = 0
	};
}

void GP0::ImageStore() {
	// TODO: image store
}

void GP0::SetDrawingAreaTopLeft() {
	auto& params = m_GPU->GetParams();
	u32 word = m_Command[0];

	params.drawing_area_top = (word >> 10) & 0x3ff;
	params.drawing_area_left = word & 0x3ff;
}

void GP0::SetDrawingAreaBottomRight() {
	auto& params = m_GPU->GetParams();
	u32 word = m_Command[0];

	params.drawing_area_bottom = (word >> 10) & 0x3ff;
	params.drawing_area_right = word & 0x3ff;
}

void GP0::SetDrawingOffset() {
	auto& params = m_GPU->GetParams();
	u32 word = m_Command[0];
	u16 x = word & 0x7ff;
	u16 y = (word >> 11) & 0x7ff;

	params.drawing_x_offset = (i16)(x << 5) >> 5;
	params.drawing_y_offset = (i16)(y << 5) >> 5;
}

void GP0::TextureWindowSetting() {
	auto& params = m_GPU->GetParams();
	u32 word = m_Command[0];

	params.texture_window_x_mask = word & 0x1f;
	params.texture_window_y_mask = (word >> 5) & 0x1f;
	params.texture_window_x_offset = (word >> 10) & 0x1f;
	params.texture_window_y_offset = (word >> 15) & 0x1f;
}

void GP0::MaskBitSetting() {
	auto& params = m_GPU->GetParams();
	u32 word = m_Command[0];

	params.force_set_mask_bit = word & 1;
	params.preserve_masked_pixels = (word >> 1) & 1;
}

void GP0::RenderPolygon() {
	std::vector<Position> positions;
	std::vector<Color> colors;
	std::vector<UV> uvs;

	size_t index = 0;
	size_t vertex_count = (m_DrawPolygonOptions.polygon == PolygonType::Quad) ? 4 : 3;

	// first color (always present in command word)
	Color first_color = Color(m_Command[index++]);

	for (size_t i = 0; i < vertex_count; i++) {
		if ((m_DrawPolygonOptions.shading == ShadingType::Gouraud) && i != 0) {
			colors.push_back(Color(m_Command[index++]));
		} else {
			colors.push_back(first_color);
		}

		positions.push_back(Position(m_Command[index++]));

		if (m_DrawPolygonOptions.texture == TextureType::Textured) {
			uvs.push_back(UV(m_Command[index++]));
		}
	}

	m_GPU->GetRenderer().DrawPolygon(m_DrawPolygonOptions, positions, colors, uvs);
}

void GP0::DecodeOp(u32 word) {
	u8 op = (word >> 24) & 0xff;

	// draw polygon
	if (((op & 0b11100000) >> 5) == 0b001) {
		m_DrawPolygonOptions = {
			.shading		= (ShadingType)			((word & (1 << 28)) ? 1 : 0),
			.polygon		= (PolygonType)			((word & (1 << 27)) ? 1 : 0),
			.texture		= (TextureType)			((word & (1 << 26)) ? 1 : 0),
			.transparency	= (TransparencyType)	((word & (1 << 25)) ? 1 : 0),
			.modulation		= (ModulationType)		((word & (1 << 24)) ? 1 : 0),
		};

		switch (m_DrawPolygonOptions.polygon) {
			case PolygonType::Triangle: {
				m_Decoded.operands = 3;																// 3 vertices
				if (m_DrawPolygonOptions.shading == ShadingType::Gouraud) m_Decoded.operands += 2;	// first color is stored in word 0, so it isnt included
				if (m_DrawPolygonOptions.texture == TextureType::Textured) m_Decoded.operands += 3;	// UV coordinates will always be a new word
				break;
			}

			case PolygonType::Quad: {
				m_Decoded.operands = 4;																// 4 vertices
				if (m_DrawPolygonOptions.shading == ShadingType::Gouraud) m_Decoded.operands += 3;	// first color is stored in word 0, so it isnt included
				if (m_DrawPolygonOptions.texture == TextureType::Textured) m_Decoded.operands += 4;	// UV coordinates will always be a new word
				break;
			}
		}

		m_Decoded.ptr = &GP0::RenderPolygon;
		return;
	}

	switch (op) {
		case 0x00: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::NOP; break; }
		case 0x01: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::ClearCache; break; }
		case 0xa0: { m_Decoded.operands = 2; m_Decoded.ptr = &GP0::ImageLoad; break; }
		case 0xc0: { m_Decoded.operands = 2; m_Decoded.ptr = &GP0::ImageStore; break; }
		case 0xe1: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::DrawModeSetting; break; }
		case 0xe2: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::TextureWindowSetting; break; }
		case 0xe3: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::SetDrawingAreaTopLeft; break; }
		case 0xe4: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::SetDrawingAreaBottomRight; break; }
		case 0xe5: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::SetDrawingOffset; break; }
		case 0xe6: { m_Decoded.operands = 0; m_Decoded.ptr = &GP0::MaskBitSetting; break; }
		
		default: {
			error_log("GP0({:02x}h) unknown", op);
			exit(1);
		}
	}
}