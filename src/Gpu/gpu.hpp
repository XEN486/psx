#ifndef GPU_GPU_HPP
#define GPU_GPU_HPP

#include "Renderer/renderer.hpp"
#include "Gp0/gp0.hpp"
#include "Gp1/gp1.hpp"

#include "../Dma/dma.hpp"
#include "../utils.hpp"

#include <vector>

namespace Gpu {
	enum class TextureDepth : u8 {
		T4 = 0,
		T8 = 1,
		T15 = 2
	};

	enum class Field : u8 {
		Top = 1,
		Bottom = 0,
	};

	struct HorizontalResolution {
		u8 raw;

		void SetFromFields(u8 hr1, u8 hr2) {
			raw = (hr2 & 1) | ((hr1 & 3) << 1);
		}

		u32 GetStatus() {
			return raw << 16;
		}

		u16 GetPixels() const {
			if (raw & 0b001) return 368;
			switch(raw) {
				case 0b000: return 256;
				case 0b010: return 320;
				case 0b100: return 512;
				case 0b110: return 640;
				default: return 320; // fallback
			}
		}
	};

	enum class VerticalResolution : u8 {
		Y240 = 0,
		Y480 = 1,
	};

	enum class VideoMode : u8 {
		NTSC = 0, // 480i60hz
		PAL = 1, // 576i50hz
	};

	enum class DisplayDepth : u8 {
		D15 = 0,
		D24 = 1,
	};

	enum class DMADirection : u8 {
		Off = 0,
		FIFO = 1,
		CPUtoGP0 = 2,
		VRAMtoCPU = 3,
	};

	struct GPUParameters {
		u8 page_base_x;
		u8 page_base_y;
		u8 semi_transparency;

		TextureDepth texture_depth;
		
		bool dithering;
		bool draw_to_display;
		bool force_set_mask_bit;
		bool preserve_masked_pixels;

		Field field;

		bool texture_disable;

		HorizontalResolution horizontal_resolution;
		VerticalResolution vertical_resolution;
		VideoMode video_mode;
		DisplayDepth display_depth;

		bool interlaced;
		bool display_disabled;
		bool interrupt;

		DMADirection dma_direction;

		bool even; // 0 = even/vblank, 1 = odd

		bool textured_rectangle_x_flip;
		bool textured_rectangle_y_flip;

		u8 texture_window_x_mask;
		u8 texture_window_y_mask;
		u8 texture_window_x_offset;
		u8 texture_window_y_offset;

		u16 drawing_area_left;
		u16 drawing_area_top;
		u16 drawing_area_right;
		u16 drawing_area_bottom;

		i16 drawing_x_offset;
		i16 drawing_y_offset;

		u16 display_vram_x_start;
		u16 display_vram_y_start;
		u16 display_horiz_start;
		u16 display_horiz_end;
		u16 display_line_start;
		u16 display_line_end;
	};

	class GPU {
	public:
		GPU(IRenderer* renderer) : m_Renderer(renderer) {
			m_Renderer->m_Params = &params;
			m_GP0.m_GPU = this;
			m_GP1.m_GPU = this;
		}

		void Reset();
		void Tick(u64 cycles);

		// CPU <-> GPU memory-mapped bus (32-bit only)
		u32 VBusRead(u32 address);
		void VBusWrite(u32 address, u32 word);

		GPUParameters& GetParams() { return params; }
		u32& GetRead() { return read; }

		GP0& GetGP0() { return m_GP0; }
		GP1& GetGP1() { return m_GP1; }
		IRenderer& GetRenderer() { return *m_Renderer; }

	public:

	private:
		u32 GetStatus();

	private:
		GP0 m_GP0;
		GP1 m_GP1;
		IRenderer* m_Renderer;

		bool m_VBlank = false;
		u64 m_DotClock = 0;
		u32 m_Scanline = 0;

		GPUParameters params;
		u32 read;
	};
}

namespace Dma {
	struct GPU : public Dma::Channel {
		GPU(Gpu::GPU* gpu) : m_GPU(gpu) {}

		void Write(u32 word) override {
			m_GPU->VBusWrite(0x1f801810, word);
		}

		u32 Read(u32, u32) {
			return m_GPU->VBusRead(0x1f801810);
		}

	private:
		Gpu::GPU* m_GPU;
	};
}

#endif