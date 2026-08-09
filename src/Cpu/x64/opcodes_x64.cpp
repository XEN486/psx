#include "jit_x64.hpp"
using namespace Cpu;
using namespace asmjit;

static u32 WRAP_ReadCOP0(R3000A* r3000a, u8 reg) { return r3000a->ReadCOP0(reg); }
static void WRAP_WriteCOP0(R3000A* r3000a, u8 reg, u32 val) { r3000a->WriteCOP0(reg, val); }

void JitX64::LUI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], (data.imm << 16));
}

void JitX64::ORI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], r[data.rs]);
	cc.or_(r[data.rt], data.imm);
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

void JitX64::LHU(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	if (data.rt == 0) {
		EmitReadVirtualMemory<u16>(temp.r16(), temp);
		return;
	}

	cc.xor_(r[data.rt], r[data.rt]);
	EmitReadVirtualMemory<u16>(r[data.rt].r16(), temp);
}

void JitX64::OR(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rs]);
	cc.or_(r[data.rd], r[data.rt]);
}

void JitX64::MTC0(InstructionData& data) {
	InvokeNode* node;

	cc.invoke(Out(node), reinterpret_cast<uintptr_t>(&WRAP_WriteCOP0), FuncSignature::build<void, R3000A*, u8, u32>());
	node->set_arg(0, r3000a);
	node->set_arg(1, data.rd);
	node->set_arg(2, r[data.rt]);
}

void JitX64::BNE(InstructionData& data) {
	Label end = cc.new_label();
	cc.cmp(r[data.rs], r[data.rt]);
	cc.j(x86::CondCode::kEqual, end);

	EmitJump((data.pc + 4) + (i32)((i16)data.imm << 2));

	cc.bind(end);
	EmitBranchDelay(data);
}

void JitX64::ADDI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], r[data.rs]);
	cc.add(r[data.rt], (i32)(i16)data.imm);
	// overflow exception
}