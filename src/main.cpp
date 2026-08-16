#include "gpu/renderer/renderer_sdl.hpp"
#include "cpu/x64/jit_x64.hpp"
#include "psx.hpp"

#include <print>

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::println(stderr, "failed to initialize sdl: {}", SDL_GetError());
		return 1;
	}

	SDL_Window* window;
	SDL_Renderer* sdl_renderer;

#ifdef ENABLE_VRAM_VIEW
	SDL_CreateWindowAndRenderer("psx", 1024, 512, SDL_WINDOW_RESIZABLE, &window, &sdl_renderer);
#else
	SDL_CreateWindowAndRenderer("psx", 1280, 960, SDL_WINDOW_RESIZABLE, &window, &sdl_renderer);
#endif

	Gpu::RendererSDL renderer(window, sdl_renderer);
	Cpu::JitX64 backend;
	
	PlayStation psx;
	psx.Create(&backend, &renderer);
	psx.LoadBIOS("roms/scph1001.bin");
	psx.Reset();

	psx.GetCPU().SideloadExe("roms/psxtest_cpu.exe");

	bool running = true;
	SDL_Event event;
	while (running) {
		psx.RunBatch();

		if (renderer.FrameReady()) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_QUIT) running = false;
			}

			renderer.Draw();
		}
	}
	
	psx.Release();
	return 0;
}