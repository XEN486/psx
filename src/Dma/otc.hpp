#ifndef DMA_OTC_HPP
#define DMA_OTC_HPP

#include "dma.hpp"

namespace Dma {
	struct OTC : public Channel {
		void Write(u32 word) override;
		u32 Read(u32 address, u32 remaining_words) override;
	};
}

#endif