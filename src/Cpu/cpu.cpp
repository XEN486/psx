#include "cpu.hpp"
#include <fstream>

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

void CPU::SideloadExe(std::filesystem::path path) {
	std::ifstream file;
	file.open(path, std::ios::binary);

	// run CPU as normal until we reach $800300000
	while (m_R3000A.pc != 0x80030000) RunOnce();

	u32 initial_pc;
	u32 initial_r28;
	u32 exe_ram_addr;
	u32 exe_size;
	u32 initial_sp;

	// read fields
	file.seekg(0x10);
	file.read(reinterpret_cast<char*>(&initial_pc), 4);
	file.read(reinterpret_cast<char*>(&initial_r28), 4);
	file.read(reinterpret_cast<char*>(&exe_ram_addr), 4);
	file.read(reinterpret_cast<char*>(&exe_size), 4);

	file.seekg(0x30);
	file.read(reinterpret_cast<char*>(&initial_sp), 4);

	// copy EXE code/data to RAM
	// ram[exe_ram_addr..(exe_ram_addr+exe_size)] <- file[2048..(2048+exe_size)]
	file.seekg(2048);
	file.read(reinterpret_cast<char*>(m_Memory.ram + (exe_ram_addr & 0x1fffffff)), exe_size);

	// set registers
	m_R3000A.gpr[28] = initial_r28;
	if (initial_sp) {
		m_R3000A.gpr[29] = initial_sp;
		m_R3000A.gpr[30] = initial_sp;
	}

	// jump to pc
	m_R3000A.pc = initial_pc;
	m_JitBackend->InvalidateAll(); // invalidate the whole block cache just in case
	
	debug_log("loaded {} ({}KiB -> {:08x})", path.filename().string(), exe_size / KiB, exe_ram_addr);
	file.close();
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

void R3000A::Exception(ExceptionCause cause, u32 epc, bool in_delay_slot) {
	// EPC <- epc
	cop0[EPC] = epc;

	// CAUSE.ExcCode <- exception cause
	u8 exc_code = 0b1111 << 2;
	cop0[CAUSE] = (cop0[CAUSE] & ~exc_code) | ((static_cast<u8>(cause) & 0b1111) << 2);

	// CAUSE.BD <- in branch delay
	u32 bd = 1UL << 31;
	cop0[CAUSE] = (cop0[CAUSE] & ~bd) | ((in_delay_slot ? 1 : 0) << 31);

	// exception in delay slot case
	if (in_delay_slot) {
		cop0[EPC] -= 4;			// EPC <- branch instruction
		cop0[TAR] = next_pc;	// TAR <- branch target
	}

	// previous <- current, and kernel mode and disable interrupts by clearing low bits
	u8 mode = cop0[SR] & 0x3f;
	cop0[SR] = (cop0[SR] & ~(u32)0x3f) | ((mode << 2) & 0x3f);

	// next pc <- exception vector
	bool bev = cop0[SR] & (1 << 22);
	next_pc = bev ? 0xbfc00180 : 0x80000080;
}