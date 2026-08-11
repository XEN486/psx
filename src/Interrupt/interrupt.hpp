#ifndef INTERRUPT_INTERRUPT_HPP
#define INTERRUPT_INTERRUPT_HPP

#include "../utils.hpp"

namespace Cpu { class CPU; }
namespace Interrupt {
	enum class IRQ : u8 {
		VBlank,
		GPU,
		CDROM,
		DMA,
		Timer0,
		Timer1,
		Timer2,
		Controller,
		SIO,
		SPU,
		Lightpen,
	};

	class INTC {
	public:
		INTC(Cpu::CPU* cpu) : m_CPU(cpu) {
			Reset();
		}

		void Reset() {
			m_STAT = 0;
			m_MASK = 0;
		}

		/// @brief Sends an IRQ to the CPU.
		void Interrupt(IRQ irq);

		/// @brief Writes to I_STAT.
		/// @param word Word to write to I_STAT.
		void SetSTAT(u32 word) {
			m_STAT &= word; // bit=0 -> clear, bit=1 -> no change
		}

		/// @brief Writes to I_MASK.
		/// @param word Word to write to I_MASK.
		void SetMASK(u32 word) {
			m_MASK = word;
			TryInterrupt();
		}

		u32 GetSTAT() const { return m_STAT; }
		u32 GetMASK() const { return m_MASK; }

	private:
		void TryInterrupt();

	private:
		Cpu::CPU* m_CPU;
		u32 m_STAT = 0;
		u32 m_MASK = 0;
	};
}

#endif