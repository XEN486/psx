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

	if ((m_R3000A.pc == 0xa0 && m_R3000A.gpr[9] == 0x3c) || (m_R3000A.pc == 0xb0 && m_R3000A.gpr[9] == 0x3d)) {
		std::print("{}", (char)(m_R3000A.gpr[4] & 0xff));
	}

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
	memset(&m_R3000A.cop0, 0, 32 * sizeof(u32));

	m_JitBackend->Reset();
}

void CPU::Release() {
	m_JitBackend->Release();
	m_Memory.Release();
}

u32 R3000A::ReadCOP0(u8 reg) {
	//debug_log("read <- cop0[{}]", reg);
	return cop0[reg];
}

void R3000A::WriteCOP0(u8 reg, u32 word) {
	//debug_log("write cop0[{}] <- {:08x}", reg, word);

	// only bit 8/9 are R/W
	if (reg == CAUSE) {
		u32 mask = 0b11 << 8;
		cop0[CAUSE] = (cop0[CAUSE] & (~mask)) | (word & mask);
	}

	if (reg == BPC || reg == BDA || reg == DCIC || reg == BDAM || reg == BPCM || reg == SR) {
		cop0[reg] = word;
	}
}