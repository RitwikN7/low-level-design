#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <optional>
#include <utility>

template <typename T, std::size_t Capacity>
class RingBuffer
{
    static_assert(Capacity > 0, "Capacity must be greater than 0");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

#if defined(__cpp_lib_hardware_interference_size)
    static constexpr std::size_t CacheLineSize = std::hardware_destructive_interference_size;
#else
    static constexpr std::size_t CacheLineSize = 64;
#endif

public:
    RingBuffer() = default;

    ~RingBuffer()
    {
        T discard;
        while (pop(discard))
        {
        }
    }

    // Non-copyable
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    // Non-movable
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    template <typename... Args>
    bool emplace(Args&&... args)
    {
        const auto head = head_.load(std::memory_order_relaxed);

        if (head - cached_tail_ == Capacity) [[unlikely]]
        {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (head - cached_tail_ == Capacity)
                return false;
        }

        auto* slot = reinterpret_cast<T*>(&storage_[(head & BufferMask) * sizeof(T)]);
        ::new (static_cast<void*>(slot)) T(std::forward<Args>(args)...);

        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool write(const T& value)
    {
        return emplace(value);
    }

    bool write(T&& value)
    {
        return emplace(std::move(value));
    }

    bool pop(T& value)
    {
        const auto tail = tail_.load(std::memory_order_relaxed);

        if (tail == cached_head_) [[unlikely]]
        {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (tail == cached_head_)
                return false;
        }

        auto* slot = reinterpret_cast<T*>(&storage_[(tail & BufferMask) * sizeof(T)]);
        value = std::move(*slot);
        slot->~T();

        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> read()
    {
        T val;
        if (pop(val))
            return val;

        return std::nullopt;
    }

private:
    static constexpr std::size_t BufferMask = Capacity - 1;

    // Producer cacheline
    alignas(CacheLineSize) std::atomic<std::size_t> head_{0};
    std::size_t cached_tail_{0};

    // Consumer cacheline
    alignas(CacheLineSize) std::atomic<std::size_t> tail_{0};
    std::size_t cached_head_{0};

    alignas(CacheLineSize) alignas(alignof(T)) std::array<std::byte, Capacity * sizeof(T)> storage_;
};