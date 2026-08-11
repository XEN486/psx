#ifndef PSX_HPP
#define PSX_HPP

#include "Gpu/gpu.hpp"
#include "Cpu/cpu.hpp"
#include "Cpu/x64/jit_x64.hpp"
#include "Dma/dma.hpp"
#include "Dma/otc.hpp"

class PlayStation {
public:
	void Create(Cpu::JitBackend* backend, Gpu::IRenderer* renderer);
	void LoadBIOS(std::filesystem::path path);
	void Reset();
	void RunBatch(u64 batch_size);
	void Release();

private:
	Gpu::GPU* m_GPU;
	Cpu::CPU* m_CPU;
	Dma::DMA* m_DMA;
};

#endif