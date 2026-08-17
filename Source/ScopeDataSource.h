#pragma once

#include <array>
#include <atomic>

namespace gexex
{
    // A small fixed-size ring buffer for the mini-oscilloscopes: pushed to
    // from the audio thread (one push per sample), read from the GUI
    // thread on a ~30Hz timer (see Scope.h). Each slot is a
    // std::atomic<float> rather than a mutex/lock-free-FIFO -- that
    // guarantees no torn/UB reads even with unsynchronized timing between
    // the two threads, which is all a *visual* waveform display actually
    // needs (an occasional stale sample during a read-while-write race is
    // imperceptible; a torn 4-byte float would not be).
    template <int Size = 512>
    class ScopeDataSource
    {
    public:
        void push(float sample) noexcept
        {
            const int i = writeIndex.load(std::memory_order_relaxed);
            buffer[(size_t) i].store(sample, std::memory_order_relaxed);
            writeIndex.store((i + 1) % Size, std::memory_order_relaxed);
        }

        // Copies the buffer out in chronological order (oldest first) into
        // `dest`, which must have room for `Size` floats.
        void copyOut(float* dest) const noexcept
        {
            const int start = writeIndex.load(std::memory_order_relaxed);
            for (int i = 0; i < Size; ++i)
                dest[i] = buffer[(size_t) ((start + i) % Size)].load(std::memory_order_relaxed);
        }

        static constexpr int size = Size;

    private:
        std::array<std::atomic<float>, (size_t) Size> buffer {};
        std::atomic<int> writeIndex { 0 };
    };
}
