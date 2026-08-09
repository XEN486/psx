#include "cpu.hpp"
using namespace Cpu;

InstructionData JitBackend::AnalyzeOp(u32 instruction) {
	InstructionData data;
	data.pc = m_CompilePC - 4;
	data.type = InstructionType::Normal;
	DecodeOp(data, instruction);

	u8 op = (instruction >> 26) & 0b111111;
	switch (op) {
		default: {
			error_log("unknown opcode {:06b} @ {:08x}", op, data.pc);
			exit(1);
		}
	}

	return data;
}