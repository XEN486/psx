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
	cc.movzx(r[data.rt], r[data.rt].r16());
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

void JitX64::LW(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	if (data.rt == 0) {
		EmitReadVirtualMemory<u32>(temp, temp);
		return;
	}

	EmitReadVirtualMemory<u32>(r[data.rt], temp);
}

void JitX64::SLTU(InstructionData& data) {
	if (data.rd == 0) return;
	cc.cmp(r[data.rs], r[data.rt]);
	cc.set(x86::CondCode::kUnsignedLT, r[data.rd].r8());
	cc.movzx(r[data.rd], r[data.rd].r8());
}

void JitX64::ADDU(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rs]);
	cc.add(r[data.rd], r[data.rt]);
}

void JitX64::SH(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	EmitWriteVirtualMemory<u16>(temp, r[data.rt].r16());
}

void JitX64::JAL(InstructionData& data) {
	cc.mov(r[31], data.pc + 8);
	EmitJump(((data.pc + 4) & 0xf0000000) | (data.addr << 2));
	EmitBranchDelay(data);
}

void JitX64::ANDI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.mov(r[data.rt], r[data.rs]);
	cc.and_(r[data.rt], data.imm);
}

void JitX64::SB(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	EmitWriteVirtualMemory<u8>(temp, r[data.rt].r8());
}

void JitX64::JR(InstructionData& data) {
	EmitJump(r[data.rs]);
	EmitBranchDelay(data);
}

void JitX64::LB(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	if (data.rt == 0) {
		EmitReadVirtualMemory<u8>(temp.r8(), temp);
		return;
	}

	EmitReadVirtualMemory<u8>(r[data.rt].r8(), temp);
	cc.movsx(r[data.rt], r[data.rt].r8());
}

void JitX64::SLLV(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(temp, r[data.rs]);
	cc.and_(temp, 0b11111);
	cc.mov(r[data.rd], r[data.rt]);
	cc.shl(r[data.rd], temp);
}

void JitX64::BEQ(InstructionData& data) {
	Label end = cc.new_label();
	cc.cmp(r[data.rs], r[data.rt]);
	cc.j(x86::CondCode::kNotEqual, end);

	EmitJump((data.pc + 4) + (i32)((i16)data.imm << 2));

	cc.bind(end);
	EmitBranchDelay(data);
}

void JitX64::MFC0(InstructionData& data) {
	InvokeNode* node;

	cc.invoke(Out(node), reinterpret_cast<uintptr_t>(&WRAP_WriteCOP0), FuncSignature::build<u32, R3000A*, u8>());
	node->set_arg(0, r3000a);
	node->set_arg(1, data.rd);
	node->set_ret(0, r[data.rt]);
}

void JitX64::AND(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rs]);
	cc.and_(r[data.rd], r[data.rt]);
}

void JitX64::ADD(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rs]);
	cc.add(r[data.rd], r[data.rt]);
	// overflow exception
}

void JitX64::BGTZ(InstructionData& data) {
	Label end = cc.new_label();
	cc.cmp(r[data.rs], 0);
	cc.j(x86::CondCode::kSignedLE, end);

	EmitJump((data.pc + 4) + (i32)((i16)data.imm << 2));

	cc.bind(end);
	EmitBranchDelay(data);
}

void JitX64::BLEZ(InstructionData& data) {
	Label end = cc.new_label();
	cc.cmp(r[data.rs], 0);
	cc.j(x86::CondCode::kSignedGT, end);

	EmitJump((data.pc + 4) + (i32)((i16)data.imm << 2));

	cc.bind(end);
	EmitBranchDelay(data);
}

void JitX64::LBU(InstructionData& data) {
	cc.mov(temp, r[data.rs]);
	if (data.imm) cc.add(temp, (i32)(i16)data.imm);
	if (data.rt == 0) {
		EmitReadVirtualMemory<u8>(temp.r8(), temp);
		return;
	}

	EmitReadVirtualMemory<u8>(r[data.rt].r8(), temp);
	cc.movzx(r[data.rt], r[data.rt].r8());
}

void JitX64::JALR(InstructionData& data) {
	EmitJump(r[data.rs]);
	if (data.rd) cc.mov(r[data.rd], data.pc + 8);
	EmitBranchDelay(data);
}

void JitX64::BLTZ(InstructionData& data) {
	Label end = cc.new_label();
	cc.cmp(r[data.rs], 0);
	cc.j(x86::CondCode::kSignedGE, end);

	EmitJump((data.pc + 4) + (i32)((i16)data.imm << 2));

	cc.bind(end);
	EmitBranchDelay(data);
}

void JitX64::SLTI(InstructionData& data) {
	if (data.rt == 0) return;
	cc.cmp(r[data.rs], (i32)(i16)data.imm);
	cc.set(x86::CondCode::kSignedLT, r[data.rt].r8());
	cc.movzx(r[data.rt], r[data.rt].r8());
}

void JitX64::SUBU(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rs]);
	cc.sub(r[data.rd], r[data.rt]);
}

void JitX64::SRA(InstructionData& data) {
	if (data.rd == 0) return;
	cc.mov(r[data.rd], r[data.rt]);
	cc.sar(r[data.rd], data.sa);
}