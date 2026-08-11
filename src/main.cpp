#include "Gpu/gpu.hpp"
#include "Cpu/cpu.hpp"
#include "Cpu/x64/jit_x64.hpp"
#include <print>

int main() {
	Gpu::GPU gpu;
	Cpu::JitX64 backend;
	Cpu::CPU cpu(&backend);
	
	gpu.Reset();
	cpu.GetMemory().Initialize(&backend, &gpu);
	cpu.GetMemory().LoadBIOS("roms/scph1001.bin");
	
	cpu.Reset();
	//cpu.SideloadExe("roms/psxtest_cpu.exe");

	while (true) {
		cpu.RunOnce();
	}
	
	cpu.Release();
	return 0;
}