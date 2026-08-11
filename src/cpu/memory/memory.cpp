#include "memory.hpp"
#include "../cpu.hpp"
#include "../../gpu/gpu.hpp"
#include "../../dma/dma.hpp"
#include "../../interrupt/interrupt.hpp"

#include <fstream>
using namespace Cpu;

#define IN_SCRATCHPAD(address) ((address >= 0x1f800000 && address <= 0x1f8003ff) || (address >= 0x9f800000 && address <= 0x9f8003ff))

void Memory::Initialize(JitBackend* backend, Gpu::GPU* gpu, Dma::DMA* dma, Interrupt::INTC* intc) {
	m_JitBackend = backend;
	m_GPU = gpu;
	m_DMA = dma;
	m_INTC = intc;

	// allocate memory for stuff on CPU bus
	ram			= static_cast<u8*>(malloc(2 * MiB));
	bios		= static_cast<u8*>(malloc(512 * KiB));
	scratchpad	= static_cast<u8*>(malloc(1 * KiB));
}

u32 Memory::ReadVirtualMemory32(u32 address) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		return *reinterpret_cast<u32*>(scratchpad + index);
	}

	// virtual -> physical
	address = VirtualToPhysical(address);

	// RAM
	if (address < 2 * MiB) {
		return *reinterpret_cast<u32*>(ram + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u32*>(bios + index);
	}

	// VBUS
	if (address == 0x1f801810 || address == 0x1f801814) {
		return m_GPU->VBusRead(address);
	}

	// DMA
	if (address >= 0x1f801080 && address <= 0x1f8010ff) {
		return m_DMA->Read(address);
	}

	// timer stub
	if (address == 0x1f801110) return 0xffffffff;

	// INTC
	if (address == 0x1f801070) return m_INTC->GetSTAT();
	if (address == 0x1f801074) return m_INTC->GetMASK();

	error_log("32-bit read <- unknown address {:08x}", address);
	return 0;
}

void Memory::WriteVirtualMemory32(u32 address, u32 word) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		*reinterpret_cast<u32*>(scratchpad + index) = word;
		return;
	}

	// virtual -> physical
	address = VirtualToPhysical(address);

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u32*>(ram + address) = word;
		return;
	}

	// VBUS
	if (address == 0x1f801810 || address == 0x1f801814) {
		m_GPU->VBusWrite(address, word);
		return;
	}

	// DMA
	if (address >= 0x1f801080 && address <= 0x1f8010ff) {
		m_DMA->Write(address, word);
		return;
	}

	// INTC
	if (address == 0x1f801070) return m_INTC->SetSTAT(word);
	if (address == 0x1f801074) return m_INTC->SetMASK(word);

	error_log("32-bit write {:08x} -> unknown address {:08x}", word, address);
}

u16 Memory::ReadVirtualMemory16(u32 address) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		return *reinterpret_cast<u16*>(scratchpad + index);
	}

	// virtual -> physical
	address = VirtualToPhysical(address);

	// RAM
	if (address < 2 * MiB) {
		return *reinterpret_cast<u16*>(ram + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u16*>(bios + index);
	}

	// SPU
	if (address >= 0x1f801c00 && address <= 0x1f801e80) return 0;

	error_log("16-bit read <- unknown address {:08x}", address);
	return 0;
}

void Memory::WriteVirtualMemory16(u32 address, u16 hword) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		*reinterpret_cast<u16*>(scratchpad + index) = hword;
		return;
	}

	// virtual -> physical
	address = VirtualToPhysical(address);

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u16*>(ram + address) = hword;
		return;
	}

	// SPU
	if (address >= 0x1f801c00 && address <= 0x1f801e80) return;

	error_log("16-bit write {:04x} -> unknown address {:08x}", hword, address);
}

u8 Memory::ReadVirtualMemory8(u32 address) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		return *reinterpret_cast<u8*>(scratchpad + index);
	}

	// virtual -> physical
	address = VirtualToPhysical(address);

	// RAM
	if (address < 2 * MiB) {
		return *reinterpret_cast<u8*>(ram + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u8*>(bios + index);
	}

	// expansion region 1 (only on 8-bit bus)
	if (address >= 0x1f000000 && address <= 0x1f7fffff) {
		return 0xff;
	}

	error_log("8-bit read <- unknown address {:08x}", address);
	return 0;
}

void Memory::WriteVirtualMemory8(u32 address, u8 byte) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		*reinterpret_cast<u8*>(scratchpad + index) = byte;
		return;
	}

	// virtual -> physical
	address = VirtualToPhysical(address);

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u8*>(ram + address) = byte;
		return;
	}

	// expansion region 1 (only on 8-bit bus)
	if (address >= 0x1f000000 && address <= 0x1f7fffff) return;

	error_log("8-bit write {:02x} -> unknown address {:08x}", byte, address);
}

void Memory::Release() {
	free(ram);
	free(bios);
	free(scratchpad);
}

void Memory::LoadBIOS(std::filesystem::path path) {
	std::ifstream file;
	file.open(path, std::ios::binary | std::ios::ate);
	
	size_t size = file.tellg();
	if (size != 512 * KiB) {
		error_log("BIOS is not valid");
		exit(1);
	}

	file.seekg(0);
	file.read(reinterpret_cast<char*>(bios), size);
	file.close();
}