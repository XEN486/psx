#ifndef PSX_HPP
#define PSX_HPP

#include "interrupt/interrupt.hpp"
#include "gpu/gpu.hpp"
#include "cpu/cpu.hpp"
#include "dma/dma.hpp"
#include "dma/otc.hpp"

class PlayStation {
public:
	void Create(Cpu::JitBackend* backend, Gpu::IRenderer* renderer);
	void LoadBIOS(std::filesystem::path path);
	void Reset();
	void RunBatch(u64 batch_size);
	void Release();

	Cpu::CPU& GetCPU() { return *m_CPU; }

private:
	Interrupt::INTC* m_INTC;
	Gpu::GPU* m_GPU;
	Cpu::CPU* m_CPU;
	Dma::DMA* m_DMA;
};

#endif