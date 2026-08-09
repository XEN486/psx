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
		return *reinterpret_cast<u32*>(ram + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u32*>(bios + index);
	}

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
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u32*>(ram + address) = word;
		return;
	}

	error_log("32-bit write {:08x} -> unknown address {:08x}", word, address);
}

u16 Memory::ReadVirtualMemory16(u32 address) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		return *reinterpret_cast<u16*>(scratchpad + index);
	}

	// virtual -> physical
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		return *reinterpret_cast<u16*>(ram + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u16*>(bios + index);
	}

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
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u16*>(ram + address) = hword;
		return;
	}

	error_log("16-bit write {:04x} -> unknown address {:08x}", hword, address);
}

u8 Memory::ReadVirtualMemory8(u32 address) {
	if (IN_SCRATCHPAD(address)) {
		u32 index = (address & 0x1fffffff) - 0x1f800000;
		return *reinterpret_cast<u8*>(scratchpad + index);
	}

	// virtual -> physical
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		return *reinterpret_cast<u8*>(ram + address);
	}

	// BIOS
	if (address >= 0x1fc00000 && address <= 0x1fc7ffff) {
		u32 index = address - 0x1fc00000;
		return *reinterpret_cast<u8*>(bios + index);
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
	address &= 0x1fffffff;

	// RAM
	if (address < 2 * MiB) {
		*reinterpret_cast<u8*>(ram + address) = byte;
		return;
	}

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