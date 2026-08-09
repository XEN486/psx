#include "jit_x64.hpp"
using namespace Cpu;

void JitX64::LUI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], (data.imm << 16));
}

void JitX64::ORI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], r[data.rs]);
	cc.or_(r[data.rt].r16(), data.imm);
}

void JitX64::SW(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	EmitWriteVirtualMemory<u32>(temp, r[data.rt]);
}

void JitX64::SLL(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rt]);
	cc.shl(r[data.rd], data.sa);
}

void JitX64::ADDIU(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], r[data.rs]);
	cc.add(r[data.rt], (i32)(i16)data.imm);
}

void JitX64::J(InstructionData& data) {
	EmitJump(((data.pc + 4) & 0xf0000000) | (data.addr << 2));
	EmitBranchDelay(data);
}