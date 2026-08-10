#include "../gpu.hpp"
using namespace Gpu;

void GP0::Reset() {
	m_WordsLeft = 0;
	command.clear();
}

void GP0::Send(u32 word) {
	// start a new command
	if (command.empty()) {
		command.push_back(word);

		DecodeOp(word);
		m_WordsLeft = m_Decoded.operands;

		// execute now if there are no operands
		if (m_WordsLeft == 0) {
			Execute();
			command.clear();
		}

		return;
	}

	// push back data
	command.push_back(word);
	m_WordsLeft--;

	// execute if no more words are left to send
	if (m_WordsLeft == 0) {
		Execute();
		command.clear();
	}
}

void GP0::Execute() {
	m_Decoded.ptr(this);
}