#include "../gpu.hpp"
using namespace Gpu;

void GP0::Reset() {
	m_PortState = GP0PortState::Command;
	m_WordsLeft = 0;
	m_Command.clear();
}

void GP0::Send(u32 word) {
	if (m_PortState == GP0PortState::ImageLoad) {
		i16 image_x = m_ImageLoadOptions.pos.x + (m_ImageLoadOptions.current_word % m_ImageLoadOptions.width);
		i16 image_y = m_ImageLoadOptions.pos.y + (m_ImageLoadOptions.current_word / m_ImageLoadOptions.width);

		u16 pixel1 = word & 0xffff;
		u16 pixel2 = (word >> 16) & 0xffff;

		m_ImageLoadOptions.current_word += 2;

		m_GPU->GetRenderer().DrawPixel(Position(image_x, image_y), Color(pixel1));
		if (image_x + 1 < m_ImageLoadOptions.width + m_ImageLoadOptions.pos.x) {
			m_GPU->GetRenderer().DrawPixel(Position(image_x + 1, image_y), Color(pixel2));
		}

		m_WordsLeft--;
		if (m_WordsLeft == 0) {
			m_PortState = GP0PortState::Command;
		}

		return;
	}

	// start a new command
	if (m_Command.empty()) {
		m_Command.push_back(word);

		DecodeOp(word);
		m_WordsLeft = m_Decoded.operands;

		// execute now if there are no operands
		if (m_WordsLeft == 0) {
			Execute();
			m_Command.clear();
		}

		return;
	}

	// push back data
	m_Command.push_back(word);
	m_WordsLeft--;

	// execute if no more words are left to send
	if (m_WordsLeft == 0) {
		Execute();
		m_Command.clear();
	}
}

void GP0::Execute() {
	(this->*(m_Decoded.ptr))();
}