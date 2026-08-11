#include "../gpu.hpp"
using namespace Gpu;

void GP1::DmaDirection(u32 word) {
	auto& params = m_GPU->GetParams();
	params.dma_direction = static_cast<DMADirection>(word & 0b11);
}

void GP1::Reset(u32) {
	auto& params = m_GPU->GetParams();
	memset(&params, 0, sizeof(GPUParameters));
	params.display_disabled = true;
	params.interlaced = true;

	params.display_horiz_start = 0x200;
	params.display_horiz_end = 0xc00;
	params.display_line_start = 0x10;
	params.display_line_end = 0x100;

	ResetCommandBuffer(0);
}

void GP1::DisplayMode(u32 word) {
	auto& params = m_GPU->GetParams();
	params.horizontal_resolution.SetFromFields(word & 3, (word >> 6) & 1);
	params.vertical_resolution = (word & 0x4) ? VerticalResolution::Y480 : VerticalResolution::Y240;
	params.video_mode = (word & 0x8) ? VideoMode::PAL : VideoMode::NTSC;
	params.display_depth = (word & 0x10) ? DisplayDepth::D15 : DisplayDepth::D24;
	params.interlaced = word & 0x20;
}

void GP1::StartOfDisplayArea(u32 word) {
	auto& params = m_GPU->GetParams();
	params.display_vram_x_start = word & 0x3fe;
	params.display_vram_y_start = (word >> 10) & 0x1ff;
}

void GP1::HorizontalDisplayRange(u32 word) {
	auto& params = m_GPU->GetParams();
	params.display_horiz_start = word & 0xfff;
	params.display_horiz_end = (word >> 12) & 0xfff;
}

void GP1::VerticalDisplayRange(u32 word) {
	auto& params = m_GPU->GetParams();
	params.display_line_start = word & 0x3ff;
	params.display_line_end = (word >> 10) & 0x3ff;
}

void GP1::ResetCommandBuffer(u32) {
	m_GPU->GetGP0().Reset();
}

void GP1::ReadInternalRegister(u32 word) {
	auto& params = m_GPU->GetParams();
	u32& read = m_GPU->GetRead();

	params.texture_window_x_mask = word & 0x1f;
	params.texture_window_y_mask = (word >> 5) & 0x1f;
	params.texture_window_x_offset = (word >> 10) & 0x1f;
	params.texture_window_y_offset = (word >> 15) & 0x1f;

	uint8_t index = word & 0xf;
	switch (index) {
		// read texture window setting
		case 0x02: {
			read = 0;
			read |= (params.texture_window_x_mask << 0);
			read |= (params.texture_window_y_mask << 5);
			read |= (params.texture_window_x_offset << 10);
			read |= (params.texture_window_y_offset << 15);

			break;
		}

		// read draw area top left
		case 0x03: {
			read = 0;
			read |= (params.drawing_area_left << 0);
			read |= (params.drawing_area_top << 10);

			break;
		}

		// read draw area bottom right
		case 0x04: {
			read = 0;
			read |= (params.drawing_area_right << 0);
			read |= (params.drawing_area_bottom << 10);

			break;
		}

		// read draw area offset
		case 0x05: {
			read = 0;
			read |= (params.drawing_x_offset << 0);
			read |= (params.drawing_y_offset << 11);

			break;
		}

		// read GPU version
		case 0x07: {
			read = 2;
			break;
		}

		// unknown
		case 0x08: {
			read = 0;
			break;
		}

		default: break;
	}
}

void GP1::DisplayEnable(u32 word) {
	auto& params = m_GPU->GetParams();
	params.display_disabled = word & 1;
}

void GP1::AcknowledgeIRQ(u32) {
	auto& params = m_GPU->GetParams();
	params.interrupt = false;
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