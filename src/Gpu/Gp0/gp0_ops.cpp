#include "../gpu.hpp"
using namespace Gpu;

static void NOP(GP0*) {
	// nothing
}

static void DrawModeSetting(GP0* gp0) {
	GPUParameters& params = gp0->gpu->params;
	u32 word = gp0->command[0];

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

static void ClearCache(GP0*) {
	// not implemented
}

static void ImageLoad(GP0* gp0) {
	u32 resolution = gp0->command[2];
	u16 width = resolution & 0xffff;
	u16 height = (resolution >> 16) & 0xffff;
	uint32_t image_size = width * height;

	// set up GP0 port to receive image data
	gp0->words_left = (image_size + 1) / 2;
	gp0->port_state = GP0PortState::ImageLoad;

	// set up image data
	//m_ImagePosition = Position(m_GP0Command[1]);
	//m_ImageWidth = width;
	//m_ImageHeight = height;
	//m_ImageWord = 0;
}

static void ImageStore(GP0*) {
	// TODO: image store
}

void GP0::DecodeOp(u32 word) {
	u8 op = (word >> 24) & 0xff;
	switch (op) {
		case 0x00: { m_Decoded.operands = 0; m_Decoded.ptr = &NOP; break; }
		case 0x01: { m_Decoded.operands = 0; m_Decoded.ptr = &ClearCache; break; }
		case 0xa0: { m_Decoded.operands = 2; m_Decoded.ptr = &ImageLoad; break; }
		case 0xc0: { m_Decoded.operands = 2; m_Decoded.ptr = &ImageStore; break; }
		case 0xe1: { m_Decoded.operands = 0; m_Decoded.ptr = &DrawModeSetting; break; }
		
		default: {
			error_log("GP0({:02x}h) unknown", op);
			exit(1);
		}
	}
}