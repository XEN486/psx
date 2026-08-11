#include "otc.hpp"
using namespace Dma;

void OTC::Write(u32 word) {
	error_log("otc write unimplemented {:08x}", word);
	exit(1);
}

u32 OTC::Read(u32 address, u32 remaining_words) {
	// last entry is end of table marker
	if (remaining_words == 1) {
		return 0xffffff;
	}

	// pointer to previous entry
	return (address - 4) & 0x1fffff;
}