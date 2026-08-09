#include "Cpu/cpu.hpp"
#include "Cpu/x64/jit_x64.hpp"
#include <print>

int main() {
	Cpu::JitX64 backend;
	Cpu::CPU cpu(&backend);
	
	cpu.GetMemory().LoadBIOS("roms/scph1001.bin");
	cpu.Reset();

	while (true) {
		cpu.RunOnce();
	}
	
	cpu.Release();
	return 0;
}