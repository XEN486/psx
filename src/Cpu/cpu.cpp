#include "cpu.hpp"
using namespace Cpu;

CPU::CPU(JitBackend* backend) : m_JitBackend(backend) {
	if (!m_JitBackend->InitJit(&m_R3000A, &m_Memory)) {
		error_log("failed to initialize backend");
		exit(1);
	}

	m_Memory.Initialize(m_JitBackend);
}

size_t CPU::RunOnce() {
	CompiledBlock& block = m_JitBackend->GetOrCompileBlock(m_R3000A.pc);
	block.execution_count++;

	m_R3000A.next_pc = block.after_end_pc;
	block.fn();
	m_R3000A.pc = m_R3000A.next_pc;

	//if (block.execution_count == 1) {
	//	debug_log("execute new block {:04x}->{:04x} [{} instructions]", block.start_pc, block.end_pc, block.instructions);
	//}

	// assume 1 instruction = 1 clock cycle.
	//m_R3000A.cop0.count += (u32)block.instructions;

	return block.instructions;
}

void CPU::Reset() {
	for (u8 i = 0; i < 32; i++) {
		m_R3000A.gpr[i] = 0;
	}
	
	m_R3000A.pc = 0xbfc00000;

	// COP0 registers
	memset(&m_R3000A.cop0, 0, sizeof(Cop0));

	m_JitBackend->Reset();
}

void CPU::Release() {
	m_JitBackend->Release();
	m_Memory.Release();
}

u32 R3000A::ReadCOP0(u8 reg) {
	switch (reg) {
		case 12: return cop0.sr;
		
		default: {
			error_log("read unknown cop0 reg {}", reg);
			exit(1);
		}
	}
}

void R3000A::WriteCOP0(u8 reg, u32 word) {
	switch (reg) {
		case 12: { cop0.sr = word; break; }

		default: {
			error_log("{:08x} -> unknown cop0 reg {}", word, reg);
			exit(1);
		}
	}
}