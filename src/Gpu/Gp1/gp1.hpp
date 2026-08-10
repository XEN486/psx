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
	};
}

#endif