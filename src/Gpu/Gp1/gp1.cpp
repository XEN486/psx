#include "../gpu.hpp"
using namespace Gpu;

void GP1::DmaDirection(u32 word) {
	gpu->params.dma_direction = static_cast<DMADirection>(word & 0b11);
}

void GP1::Reset(u32) {
	gpu->Reset();
}

void GP1::DisplayMode(u32 word) {
	gpu->params.horizontal_resolution.SetFromFields(word & 3, (word >> 6) & 1);
	gpu->params.vertical_resolution = (word & 0x4) ? VerticalResolution::Y480 : VerticalResolution::Y240;
	gpu->params.video_mode = (word & 0x8) ? VideoMode::PAL : VideoMode::NTSC;
	gpu->params.display_depth = (word & 0x10) ? DisplayDepth::D15 : DisplayDepth::D24;
	gpu->params.interlaced = word & 0x20;
}

void GP1::Send(u32 word) {
	u8 op = (word >> 24) & 0xff;
	switch (op) {
		case 0x00: { GP1::Reset(word); break; }
		case 0x04: { GP1::DmaDirection(word); break; }
		case 0x08: { GP1::DisplayMode(word); break; }
		
		default: {
			error_log("GP1({:02x}h) unknown", op);
			exit(1);
		}
	}
}