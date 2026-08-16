#ifndef GPU_GP0_HPP
#define GPU_GP0_HPP

#include "../../utils.hpp"
#include <vector>

namespace Gpu {
	class GP0;
	struct GP0Command {
		void (GP0::*ptr)();
		size_t operands;	// number of operands
	};

	enum class GP0PortState {
		Command,
		ImageLoad,
	};

	struct ImageLoadOptions {
		Position pos;
		u32 width;
		u32 height;
		u32 current_word;
	};

	class GPU;
	class GP0 {
	public:
		void Reset();
		void Send(u32 word);

	private:
		void NOP();
		void DrawModeSetting();
		void ClearCache();
		void ImageLoad();
		void ImageStore();
		void SetDrawingAreaTopLeft();
		void SetDrawingAreaBottomRight();
		void SetDrawingOffset();
		void TextureWindowSetting();
		void MaskBitSetting();
		void RenderPolygon();
		void RenderPixel();
	
	private:
		void Execute();
		void DecodeOp(u32 word);

	private:
		GP0Command m_Decoded;
		DrawPolygonOptions m_DrawPolygonOptions;
		ImageLoadOptions m_ImageLoadOptions;

		std::vector<u32> m_Command;
		GP0PortState m_PortState;
		size_t m_WordsLeft;
		GPU* m_GPU;
		
		friend class GPU;
	};
}

#endif