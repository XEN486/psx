#include "../gpu.hpp"
using namespace Gpu;

void GP1::DmaDirection(u32 word) {
	gpu->params.dma_direction = static_cast<DMADirection>(word & 0b11);
}

void GP1::Reset(u32) {
	memset(&gpu->params, 0, sizeof(GPUParameters));
	gpu->params.display_disabled = true;
	gpu->params.interlaced = true;

	gpu->params.display_horiz_start = 0x200;
	gpu->params.display_horiz_end = 0xc00;
	gpu->params.display_line_start = 0x10;
	gpu->params.display_line_end = 0x100;

	ResetCommandBuffer(0);
	gpu->Reset();
}

void GP1::DisplayMode(u32 word) {
	gpu->params.horizontal_resolution.SetFromFields(word & 3, (word >> 6) & 1);
	gpu->params.vertical_resolution = (word & 0x4) ? VerticalResolution::Y480 : VerticalResolution::Y240;
	gpu->params.video_mode = (word & 0x8) ? VideoMode::PAL : VideoMode::NTSC;
	gpu->params.display_depth = (word & 0x10) ? DisplayDepth::D15 : DisplayDepth::D24;
	gpu->params.interlaced = word & 0x20;
}

void GP1::StartOfDisplayArea(u32 word) {
	gpu->params.display_vram_x_start = word & 0x3fe;
	gpu->params.display_vram_y_start = (word >> 10) & 0x1ff;
}

void GP1::HorizontalDisplayRange(u32 word) {
	gpu->params.display_horiz_start = word & 0xfff;
	gpu->params.display_horiz_end = (word >> 12) & 0xfff;
}

void GP1::VerticalDisplayRange(u32 word) {
	gpu->params.display_line_start = word & 0x3ff;
	gpu->params.display_line_end = (word >> 10) & 0x3ff;
}

void GP1::ResetCommandBuffer(u32) {
	gpu->GetGP0().command.clear();
	gpu->GetGP0().words_left = 0;
	gpu->GetGP0().port_state = GP0PortState::Command;
}

void GP1::ReadInternalRegister(u32 word) {
	gpu->params.texture_window_x_mask = word & 0x1f;
	gpu->params.texture_window_y_mask = (word >> 5) & 0x1f;
	gpu->params.texture_window_x_offset = (word >> 10) & 0x1f;
	gpu->params.texture_window_y_offset = (word >> 15) & 0x1f;

	uint8_t index = word & 0xf;
	switch (index) {
		// read texture window setting
		case 0x02: {
			gpu->read = 0;
			gpu->read |= (gpu->params.texture_window_x_mask << 0);
			gpu->read |= (gpu->params.texture_window_y_mask << 5);
			gpu->read |= (gpu->params.texture_window_x_offset << 10);
			gpu->read |= (gpu->params.texture_window_y_offset << 15);

			break;
		}

		// read draw area top left
		case 0x03: {
			gpu->read = 0;
			gpu->read |= (gpu->params.drawing_area_left << 0);
			gpu->read |= (gpu->params.drawing_area_top << 10);

			break;
		}

		// read draw area bottom right
		case 0x04: {
			gpu->read = 0;
			gpu->read |= (gpu->params.drawing_area_right << 0);
			gpu->read |= (gpu->params.drawing_area_bottom << 10);

			break;
		}

		// read draw area offset
		case 0x05: {
			gpu->read = 0;
			gpu->read |= (gpu->params.drawing_x_offset << 0);
			gpu->read |= (gpu->params.drawing_y_offset << 11);

			break;
		}

		// read GPU version
		case 0x07: {
			gpu->read = 2;
			break;
		}

		// unknown
		case 0x08: {
			gpu->read = 0;
			break;
		}

		default: break;
	}
}

void GP1::DisplayEnable(u32 word) {
	gpu->params.display_disabled = word & 1;
}

void GP1::AcknowledgeIRQ(u32) {
	gpu->params.interrupt = false;
}

void GP1::Send(u32 word) {
	u8 op = (word >> 24) & 0xff;
	switch (op) {
		case 0x00: { GP1::Reset(word); break; }
		case 0x01: { GP1::ResetCommandBuffer(word); break; }
		case 0x02: { GP1::AcknowledgeIRQ(word); break; }
		case 0x03: { GP1::DisplayEnable(word); break; }
		case 0x04: { GP1::DmaDirection(word); break; }
		case 0x05: { GP1::StartOfDisplayArea(word); break; }
		case 0x06: { GP1::HorizontalDisplayRange(word); break; }
		case 0x07: { GP1::VerticalDisplayRange(word); break; }
		case 0x08: { GP1::DisplayMode(word); break; }
		case 0x10: { GP1::ReadInternalRegister(word); break; }

		default: {
			error_log("GP1({:02x}h) unknown", op);
			exit(1);
		}
	}
}