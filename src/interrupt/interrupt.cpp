#include "interrupt.hpp"
#include "../Cpu/cpu.hpp"
using namespace Interrupt;

void INTC::Interrupt(IRQ irq) {
	// set bit in I_STAT
	m_STAT |= (1 << static_cast<u8>(irq));
	TryInterrupt();
}

void INTC::TryInterrupt() {
	for (u8 i = 0; i <= 10; i++) {
		u32 bit = (1 << i);

		if ((m_STAT & bit) && (m_MASK & bit)) {
			// cop0r13.10 set
			m_CPU->GetR3000A().cop0[Cpu::CAUSE] |= (1 << 10);
			return;
		}
		
		// clear cop0r13.10
		else {
			m_CPU->GetR3000A().cop0[Cpu::CAUSE] &= ~(u32)(1 << 10);
		}
	}
}