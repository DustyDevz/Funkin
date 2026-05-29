// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "input_types.hpp"

namespace Funkin::Input {
    template<size_t N>
    class InputRingBuffer {
    public:
        static_assert((N & (N - 1)) == 0, "N must be power of 2");

        bool push(const InputEvent& e) {
            size_t head = m_head.load(std::memory_order_relaxed);
            size_t next = (head + 1) & (N - 1);

            if (next == m_tail.load(std::memory_order_acquire))
                return false;

            m_buffer[head] = e;
            m_head.store(next, std::memory_order_release);
            return true;
        }

        bool pop(InputEvent& out) {
            size_t tail = m_tail.load(std::memory_order_relaxed);

            if (tail == m_head.load(std::memory_order_acquire))
                return false;

            out = m_buffer[tail];
            m_tail.store((tail + 1) & (N - 1), std::memory_order_release);
            return true;
        }

        bool empty() const {
            return m_tail.load(std::memory_order_acquire) ==
                   m_head.load(std::memory_order_acquire);
        }

    private:
        std::array<InputEvent, N>  m_buffer{};
        std::atomic<size_t>        m_head{ 0 };
        std::atomic<size_t>        m_tail{ 0 };
    };
}