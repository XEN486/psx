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

	protected:
		void LUI(InstructionData& data) override;
		void ORI(InstructionData& data) override;
		void SW(InstructionData& data) override;
		void SLL(InstructionData& data) override;
		void ADDIU(InstructionData& data) override;
		void J(InstructionData& data) override;
		void LHU(InstructionData& data) override;
		void OR(InstructionData& data) override;
		void MTC0(InstructionData& data) override;
		void BNE(InstructionData& data) override;
		void ADDI(InstructionData& data) override;
		void LW(InstructionData& data) override;
		void SLTU(InstructionData& data) override;
		
	private:
		void FlushRegisters();
		void LoadRegisters();

		template <typename T>
		void EmitJump(T address);

		template <typename Size, typename T>
		void EmitReadVirtualMemory(const asmjit::x86::Gp& ret, T address);

		template <typename Size, typename T>
		void EmitWriteVirtualMemory(T address, const asmjit::x86::Gp& value);

	private:
		asmjit::x86::Compiler cc;
		asmjit::x86::Gp r3000a;
		asmjit::x86::Gp temp;
		asmjit::x86::Gp r[32];
	};
}

#include "jit_x64.inl"
#endif