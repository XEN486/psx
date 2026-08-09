#include "cpu.hpp"
using namespace Cpu;

InstructionData JitBackend::AnalyzeOp(u32 instruction) {
	InstructionData data;
	data.pc = m_CompilePC - 4;
	data.type = InstructionType::Normal;
	DecodeOp(data, instruction);

	u8 op = (instruction >> 26) & 0b111111;
	switch (op) {
		// special
		case 0b000000: {
			switch (data.funct) {
				case 0b000000: {
					// optimize canonical NOP (SLL $0,$0,0)
					if (data.rt == 0 && data.rd == 0 && data.sa == 0) {
						data.ptr = &JitBackend::NOP;
						break;
					
						UseRegisters({data.rt, data.rd});
						data.ptr = &JitBackend::SLL;
						break;
					}
				}
				
				default: {
					error_log("unknown special opcode {:06b} @ {:08x}", data.funct, data.pc);
					exit(1);
				}
			}
			break;
		}

		// normal
		case 0b001111: { UseRegisters({data.rt}); data.ptr = &JitBackend::LUI; break; }
		case 0b001101: { UseRegisters({data.rs, data.rt}); data.ptr = &JitBackend::ORI; break; }
		case 0b101011: { UseRegisters({data.rs, data.rt}); data.ptr = &JitBackend::SW; break; }
		case 0b001001: { UseRegisters({data.rs, data.rt}); data.ptr = &JitBackend::ADDIU; break; }

		// branch
		case 0b000010: {
			data.ptr = &JitBackend::J;
			data.type = InstructionType::Branch;
			break;
		}

		default: {
			error_log("unknown opcode {:06b} @ {:08x}", op, data.pc);
			exit(1);
		}
	}

	return data;
}