#include "psx.hpp"

void PlayStation::Create(Cpu::JitBackend* backend, Gpu::IRenderer* renderer) {
	m_CPU = new Cpu::CPU(backend);
	m_INTC = new Interrupt::INTC(m_CPU);
	m_GPU = new Gpu::GPU(renderer, m_INTC);
	m_DMA = new Dma::DMA(&m_CPU->GetMemory());

	m_DMA->SetChannel(Dma::Port::GPU, std::make_shared<Dma::GPU>(m_GPU));
	m_DMA->SetChannel(Dma::Port::OTC, std::make_shared<Dma::OTC>());
	
	m_CPU->GetMemory().Initialize(backend, m_GPU, m_DMA, m_INTC);
}

void PlayStation::LoadBIOS(std::filesystem::path path) {
	m_CPU->GetMemory().LoadBIOS(path);
}

void PlayStation::Reset() {
	m_INTC->Reset();
	m_CPU->Reset();
	m_GPU->Reset();
	m_DMA->Reset();
}

void PlayStation::RunBatch(u64 batch_size) {
	u64 cpu_cycles = 0;
	while (cpu_cycles < batch_size) {
		size_t instructions = m_CPU->RunOnce();
		cpu_cycles += instructions * 2; // 2 cycles per instruction
	}

	m_GPU->Tick((cpu_cycles * 2) / 3);
}

void PlayStation::Release() {
	m_CPU->Release();
	delete m_INTC;
	delete m_CPU;
	delete m_GPU;
	delete m_DMA;
}