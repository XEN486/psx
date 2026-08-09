#include "jit_x64.hpp"
#include "../../config.hpp"

using namespace Cpu;
using namespace asmjit;

bool JitX64::InitJit(R3000A* cpu, Memory* memory) {
	if (!JitBackend::InitJit(cpu, memory)) return false;
	if (m_CodeHolder.attach(&cc) != Error::kOk) return false;

#ifdef ENABLE_ASMJIT_LOGGING
	m_Logger.set_flags(FormatFlags::kHexImms | FormatFlags::kHexOffsets | FormatFlags::kRegCasts);
	cc.add_diagnostic_options(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);
#endif
	return true;
}

void JitX64::EmitBeginBlock() {
	cc.add_func(FuncSignature::build<void>());

	r3000a = cc.new_gp_ptr("r3000a");
	temp = cc.new_gp32("temp");
	
	cc.movabs(r3000a, reinterpret_cast<uintptr_t>(m_R3000A));
	for (u8 i = 0; i < 32; i++) {
		if (!m_UsedRegisters[i]) continue;

		r[i] = cc.new_gp32(g_RegNames[i]);
		if (i == 0) {
			cc.xor_(r[i], r[i]);
			continue;
		}

		cc.mov(r[i], x86::qword_ptr(r3000a, offsetof(R3000A, gpr) + (i * sizeof(u32))));
	}
}

void JitX64::EmitEndBlock() {
	FlushRegisters();
	cc.end_func();
	cc.finalize();
}

void JitX64::FlushRegisters() {
	for (u8 i = 1; i < 32; i++) {
		if (!m_UsedRegisters[i]) continue;
		//m_UsedRegisters[i] = false;
		cc.mov(x86::qword_ptr(r3000a, offsetof(R3000A, gpr) + (i * sizeof(u32))), r[i]);
	}
}

void JitX64::LoadRegisters() {
	for (u8 i = 0; i < 32; i++) {
		if (!m_UsedRegisters[i]) continue;
		if (i == 0) {
			cc.xor_(r[i], r[i]);
			continue;
		}

		cc.mov(r[i], x86::qword_ptr(r3000a, offsetof(R3000A, gpr) + (i * sizeof(u32))));
	}
}