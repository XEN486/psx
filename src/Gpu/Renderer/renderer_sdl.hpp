#include "renderer.hpp"
#include <vector>

namespace Gpu {
	class RendererSDL : public IRenderer {
	public:
		RendererSDL(SDL_Window* window, SDL_Renderer* renderer) : m_Window(window), m_Renderer(renderer), m_VRAM(1024 * 512) {
			m_Texture = SDL_CreateTexture(
				m_Renderer,
				SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_STREAMING,
				1024, 512
			);

			SDL_SetTextureScaleMode(m_Texture, SDL_SCALEMODE_NEAREST);
		}

		u16 GetPixel(Position position) override;
		void DrawPixel(Position position, Color color) override;
		void DrawPolygon(DrawPolygonOptions& options, std::span<Position> positions, std::span<Color> colors, std::span<UV> uvs) override;
		void Draw() override;
		
	private:
		void DrawTriangle(DrawPolygonOptions& options, std::span<Position> positions, std::span<Color> colors, std::span<UV> uvs);

	private:
		SDL_Window* m_Window = nullptr;
		SDL_Renderer* m_Renderer = nullptr;
		SDL_Texture* m_Texture = nullptr;

		std::vector<uint16_t> m_VRAM;
	};
}