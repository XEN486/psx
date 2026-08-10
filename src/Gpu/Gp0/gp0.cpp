#include "../gpu.hpp"
using namespace Gpu;

void GP0::Reset() {
	port_state = GP0PortState::Command;
	words_left = 0;
	command.clear();
}

void GP0::Send(u32 word) {
	if (port_state == GP0PortState::ImageLoad) {
		// TODO: process and send to vram
		words_left--;

		if (words_left == 0) {
			port_state = GP0PortState::Command;
		}

		return;
	}

	// start a new command
	if (command.empty()) {
		command.push_back(word);

		DecodeOp(word);
		words_left = m_Decoded.operands;

		// execute now if there are no operands
		if (words_left == 0) {
			Execute();
			command.clear();
		}

		return;
	}

	// push back data
	command.push_back(word);
	words_left--;

	// execute if no more words are left to send
	if (words_left == 0) {
		Execute();
		command.clear();
	}
}

void GP0::Execute() {
	m_Decoded.ptr(this);
}