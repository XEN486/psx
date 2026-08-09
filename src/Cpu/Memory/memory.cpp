#include "memory.hpp"
#include "../cpu.hpp"

#include <fstream>
using namespace Cpu;

#define IN_SCRATCHPAD(address) ((address >= 0x1f800000 && address <= 0x1f8003ff) || (address >= 0x9f800000 && address <= 0x9f8003ff))

void Memory::Initialize(JitBackend* backend) {
	m_JitBackend = backend;

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
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		return *reinterpret_cast<u32*>(scratchpad + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u32*>(bios + index);
	}

	debug_log("read address 0x{:08x}", address);
	return 0;
}

void Memory::WriteVirtualMemory32(u32 address, u32 word) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		*reinterpret_cast<u32*>(scratchpad + index) = word;
		return;
	}

	// virtual -> physical
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u32*>(scratchpad + address) = word;
		return;
	}

	debug_log("write 0x{:08x} -> 0x{:08x}", word, address);
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