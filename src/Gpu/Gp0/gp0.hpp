#ifndef GPU_GP0_HPP
#define GPU_GP0_HPP

#include "../../utils.hpp"
#include <vector>

namespace Gpu {
	class GP0;
	struct GP0Command {
		void (*ptr)(GP0*);
		size_t operands;	// number of operands
	};

	enum class GP0PortState {
		Command,
		ImageLoad,
	};

	class GPU;
	class GP0 {
	public:
		void Reset();
		void Send(u32 word);

	public:
		std::vector<u32> command;
		GP0PortState port_state;
		size_t words_left;
		GPU* gpu;
	
	private:
		void Execute();
		void DecodeOp(u32 word);

	private:
		GP0Command m_Decoded;
	};
}

#endif