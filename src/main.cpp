#include "Gpu/Renderer/renderer_sdl.hpp"
#include "Gpu/gpu.hpp"
#include "Cpu/cpu.hpp"
#include "Cpu/x64/jit_x64.hpp"
#include "Dma/dma.hpp"
#include "Dma/otc.hpp"
#include <print>

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::println(stderr, "failed to initialize sdl: {}", SDL_GetError());
		return 1;
	}

	SDL_Window* window;
	SDL_Renderer* sdl_renderer;
	SDL_CreateWindowAndRenderer("psx", 1024, 512, SDL_WINDOW_RESIZABLE, &window, &sdl_renderer);

	Gpu::RendererSDL renderer(window, sdl_renderer);
	Gpu::GPU gpu(&renderer);
	Cpu::JitX64 backend;
	Cpu::CPU cpu(&backend);
	Dma::DMA dma(&cpu.GetMemory());

	dma.SetChannel(Dma::Port::GPU, std::make_shared<Dma::GPU>(&gpu));
	dma.SetChannel(Dma::Port::OTC, std::make_shared<Dma::OTC>());
	
	cpu.GetMemory().Initialize(&backend, &gpu, &dma);
	cpu.GetMemory().LoadBIOS("roms/scph1001.bin");

	dma.Reset();
	gpu.Reset();
	cpu.Reset();

	cpu.SideloadExe("roms/psxtest_cpu.exe");

	constexpr u64 CPU_HZ = 33'868'800;
	constexpr u64 FRAMES_PER_BATCH = 4;
	constexpr u64 CYCLES_PER_BATCH = CPU_HZ / (FRAMES_PER_BATCH * 60);

	bool running = true;
	SDL_Event event;
	while (running) {
		u64 cpu_cycles = 0;
		while (cpu_cycles < CYCLES_PER_BATCH) {
			size_t instructions = cpu.RunOnce();
			cpu_cycles += instructions * 2; // 2 cycles per instruction
		}

		gpu.Tick((cpu_cycles * 2) / 3);

		if (renderer.FrameReady()) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_QUIT) running = false;
			}

			renderer.Draw();
		}
	}
	
	cpu.Release();
	return 0;
}