#ifndef CPU_X64_JIT_X64_HPP
#define CPU_X64_JIT_X64_HPP

#include "../cpu.hpp"
#include <asmjit/x86.h>

namespace Cpu {
	class JitX64 : public JitBackend {
	public:
		bool InitJit(R3000A* cpu, Memory* memory) override;

	protected:
		void EmitBeginBlock() override;
		void EmitEndBlock() override;

	private:
		void FlushRegisters();
		void LoadRegisters();

	private:
		asmjit::x86::Compiler cc;
		asmjit::x86::Gp r3000a;
		asmjit::x86::Gp temp;
		asmjit::x86::Gp r[32];
	};
}

#endif