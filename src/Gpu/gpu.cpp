#include "gpu.hpp"
using namespace Gpu;

void GPU::Reset() {
	m_GP0.Reset();
	
	m_GP0.gpu = this;
	m_GP1.gpu = this;
}

u32 GPU::VBusRead(u32 address) {
	if (address == 0x1f801814) return GetStatus();
	else return GetRead();
}

void GPU::VBusWrite(u32 address, u32 word) {
	if (address == 0x1f801814) m_GP1.Send(word);
	else m_GP0.Send(word);
}

u32 GPU::GetStatus() {
	u32 r = 0;
	r |= params.page_base_x << 0;
	r |= params.page_base_y << 4;
	r |= params.semi_transparency << 5;
	r |= static_cast<u8>(params.texture_depth) << 7;
	r |= params.dithering << 9;
	r |= params.draw_to_display << 10;
	r |= params.force_set_mask_bit << 11;
	r |= params.preserve_masked_pixels << 12;
	r |= static_cast<u8>(params.field) << 13;
	// bit 14 not supported
	r |= params.texture_disable << 15;
	r |= params.horizontal_resolution.GetStatus();
	//r |= static_cast<u8>(params.vertical_resolution) << 19;
	r |= static_cast<u8>(params.video_mode) << 20;
	r |= static_cast<u8>(params.display_depth) << 21;
	r |= params.interlaced << 22;
	r |= params.display_disabled << 23;
	r |= params.interrupt << 24;

	// pretend GPU is always ready
	// 1. ready to receive command
	// 2. ready to send VRAM -> CPU
	// 3. ready to receive DMA block
	r |= 1 << 26;
	r |= 1 << 27;
	r |= 1 << 28;

	r |= static_cast<u8>(params.dma_direction) << 29;
	//r |= (!m_VBlank && params.even) << 31;

	bool dma_request = 0;
	switch (params.dma_direction) {
		case DMADirection::Off: dma_request = 0; break;
		case DMADirection::FIFO: dma_request = 1; break;
		case DMADirection::CPUtoGP0: dma_request = (r >> 28) & 1; break;
		case DMADirection::VRAMtoCPU: dma_request = (r >> 27) & 1; break;
	}

	r |= dma_request << 25;
	return 0x5e800000; // hardcode for now
}

u32 GPU::GetRead() {
	return 0;
}