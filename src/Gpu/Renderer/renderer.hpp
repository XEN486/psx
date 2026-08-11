#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "../../utils.hpp"
#include <SDL3/SDL.h>

#include <span>

namespace Gpu {
	struct Position {
		Position() : x(0), y(0) {}
		Position(i16 x, i16 y) : x(x), y(y) {}
		Position(u32 val) : x(val & 0xffff), y((val >> 16) & 0xffff) {}
		Position(u32 val, i16 x_offset, i16 y_offset) : x((val & 0xffff) + x_offset), y(((val >> 16) & 0xffff) + y_offset) {}

		i16 x;
		i16 y;
	};

	struct Color {
		Color(u32 val)
			: r(val & 0xff), g((val >> 8) & 0xff), b((val >> 16) & 0xff), b15(((val >> 24) & 0xff) != 0) {}

		Color(u16 rgb555) {
			r = uint8_t((((rgb555 & 0b11111) * 527u) + 23u) >> 6);
			g = uint8_t(((((rgb555 >> 5) & 0b11111) * 527u) + 23u) >> 6);
			b = uint8_t(((((rgb555 >> 10) & 0b11111) * 527u) + 23u) >> 6);
			b15 = (rgb555 >> 15) & 1;
		}

		Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), b15(false) {}
		Color() : r(0), g(0), b(0), b15(false) {}
		
		u16 Get555() const {
			return (b15 << 15) | ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3);
		}

		u32 Get888() const {
			return (r << 16) | (g << 8) | b;
		}
		
		uint8_t r;
		uint8_t g;
		uint8_t b;
		bool b15;
	};

	struct UV {
		UV(u32 val) : extra((val >> 16) & 0xffff), v((val >> 8) & 0xff), u(val & 0xff) {}
		UV(u16 val) : extra(0), v((val >> 8) & 0xff), u(val & 0xff) {}
		UV(u16 extra, uint8_t u, uint8_t v) : extra(extra), v(v), u(u) {}

		u16 extra;
		uint8_t u;
		uint8_t v;
	};

	enum class ShadingType {
		Flat,
		Gouraud,
	};

	enum class PolygonType {
		Triangle,
		Quad,
	};

	enum class TextureType {
		Untextured,
		Textured,
	};

	enum class TransparencyType {
		Opaque,
		SemiTransparent,
	};

	enum class ModulationType {
		Modulated,
		Raw,
	};

	struct DrawPolygonOptions {
		ShadingType shading;
		PolygonType polygon;
		TextureType texture;
		TransparencyType transparency;
		ModulationType modulation;
	};

	enum class RectangleSize {
		VariableSize,
		SinglePixel,
		Sprite8x8,
		Sprite16x16
	};

	struct DrawRectangleOptions {
		RectangleSize size;
		TextureType texture;
		TransparencyType transparency;
		ModulationType modulation;
	};

	struct GPUParameters;
	class IRenderer {
	public:
		virtual u16 GetPixel(Position position) = 0;
		virtual void DrawPixel(Position position, Color color) = 0;
		virtual void DrawPolygon(DrawPolygonOptions& options, std::span<Position> positions, std::span<Color> colors, std::span<UV> uvs) = 0;

		virtual void Draw() = 0;

		bool FrameReady() {
			bool done = m_FrameDone;

			m_FrameDone = false;
			return done;
		}

		void SetFrameReady() {
			m_FrameDone = true;
		}

	protected:
		GPUParameters* m_Params;
		bool m_FrameDone = false;
		friend class GPU;
	};
}

#endif