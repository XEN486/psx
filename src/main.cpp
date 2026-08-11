#include "Gpu/gpu.hpp"
#include "Cpu/cpu.hpp"
#include "Cpu/x64/jit_x64.hpp"
#include "Dma/dma.hpp"
#include "Dma/otc.hpp"
#include <print>

int main() {
	Gpu::GPU gpu;
	Cpu::JitX64 backend;
	Cpu::CPU cpu(&backend);
	Dma::DMA dma(&cpu.GetMemory());

	dma.SetChannel(Dma::Port::GPU, std::make_shared<Dma::GPU>(&gpu));
	dma.SetChannel(Dma::Port::OTC, std::make_shared<Dma::OTC>());
	
	dma.Reset();
	gpu.Reset();
	cpu.GetMemory().Initialize(&backend, &gpu, &dma);
	cpu.GetMemory().LoadBIOS("roms/scph1001.bin");
	
	cpu.Reset();
	//cpu.SideloadExe("roms/psxtest_cpu.exe");

	while (true) {
		cpu.RunOnce();
	}
	
	cpu.Release();
	return 0;
}