#ifndef MEMORY_MEMORY_HPP
#define MEMORY_MEMORY_HPP

#include "../../utils.hpp"
#include <filesystem>

namespace Cpu {
	static constexpr const size_t KiB = 1024;
	static constexpr const size_t MiB = 1024 * KiB; 

	class JitBackend;
	class Memory {
	public:
		/// @brief Initializes and allocates everything necessary for the memory map.
		/// @param backend Pointer to the JIT backend.
		void Initialize(JitBackend* backend);

		/// @brief Reads a 32-bit value from the specified address
		/// @param address Address to read from.
		/// @return Value at the address.
		u32 ReadVirtualMemory32(u32 address);

		/// @brief Writes a 32-bit value to the specified address
		/// @param address Address to write to.
		/// @param word Value to write.
		void WriteVirtualMemory32(u32 address, u32 word);
		
		/// @brief Releases the resources used by the memory map. Called by CPU::Release();
		void Release();

		void LoadBIOS(std::filesystem::path path);

	public:
		u8* ram;
		u8* bios;
		u8* scratchpad;

	private:
		JitBackend* m_JitBackend;
	};

}

#endif