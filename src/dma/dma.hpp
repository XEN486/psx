#ifndef DMA_DMA_HPP
#define DMA_DMA_HPP

#include "../cpu/memory/memory.hpp"
#include "../utils.hpp"

#include <memory>
#include <utility>

namespace Dma {
	enum class Direction : u8 {
		ToRam = 0,
		FromRam = 1,
	};

	enum class Step : u8 {
		Increment = 0,
		Decrement = 1,
	};

	enum class Sync : u8 {
		Manual = 0,
		Request = 1,
		LinkedList = 2,
	};

	enum class Port {
		MDECIn,
		MDECOut,
		GPU,
		CDROM,
		SPU,
		PIO,
		OTC,
	};

	struct Channel {
		virtual ~Channel() = default;
		virtual void Write(u32 word) = 0;
		virtual u32 Read(u32 address, u32 remaining_words) = 0;

		bool enable;
		Direction direction;
		Step step;
		Sync sync;
		bool trigger;
		bool chop;
		u8 chop_dma_size;
		u8 chop_cpu_size;
		u8 dummy;

		u32 base;

		u16 block_size;
		u16 block_count;

		Port port;

		u32 GetControl();
		void SetControl(u32 value);

		u32 GetBlockControl() const {
			return (block_count << 16) | block_size;
		}

		void SetBlockControl(u32 value) {
			block_size = value & 0xffff;
			block_count = (value >> 16) & 0xffff;
		}

		bool IsActive() const {
			bool triggered = (sync == Sync::Manual) ? trigger : true;
			return enable && triggered;
		}

		u32 GetTransferSize() {
			switch (sync) {
				case Sync::Manual: return block_size;
				case Sync::Request: return block_size * block_count;
				case Sync::LinkedList: return 0; // this shouldnt even be called
			}

			std::unreachable();
		}

		void TransferDone() {
			enable = false;
			trigger = false;
		}
	};

	struct InterruptRegister {
		bool enable_irq;
		u8 channel_enable_irq;
		u8 channel_irq_flags;
		bool force_irq;
		u8 dummy;

		u32 GetValue();
		void SetValue(u32 value);
		bool GetIRQStatus();
	};

	class DMA {
	public:
		DMA(Cpu::Memory* memory) : m_Memory(memory) {
			Reset();
		}

		void Reset();

		u32 Read(u32 address);
		void Write(u32 address, u32 word);

		std::shared_ptr<Channel> GetChannel(Port port) {
			return m_Channels[static_cast<u8>(port)];
		}

		void SetChannel(Port port, std::shared_ptr<Channel> channel) {
			channel->port = port;
			m_Channels[static_cast<u8>(port)] = channel;
		}

	private:
		void DoDMATransfer(std::shared_ptr<Channel> channel);
		void DoBlockCopy(std::shared_ptr<Channel> channel);
		void DoLinkedList(std::shared_ptr<Channel> channel);

	private:
		u32 m_Control;

		Cpu::Memory* m_Memory;
		std::shared_ptr<Channel> m_Channels[7];
		InterruptRegister m_Interrupt;
	};
}

#endif