#ifndef GPU_GP1_HPP
#define GPU_GP1_HPP

#include "../../utils.hpp"

namespace Gpu {
	class GPU;
	class GP1 {
	public:
		void Send(u32 word);

	public:
		GPU* gpu;

	private:
		void DmaDirection(u32 word);
		void Reset(u32 word);
		void DisplayMode(u32 word);
		void StartOfDisplayArea(u32 word);
		void HorizontalDisplayRange(u32 word);
		void VerticalDisplayRange(u32 word);
		void ResetCommandBuffer(u32 word);
		void ReadInternalRegister(u32 word);
		void DisplayEnable(u32 word);
		void AcknowledgeIRQ(u32 word);
	};
}

#endif