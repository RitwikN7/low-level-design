/*
Requirements:
    1. A fixed size free list allocator that supports malloc and free
    2. Allow different searching strategies (best-fit, worst-fit, first-fit, next-fit)
    3. Manage headers and lists within free space
*/

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>

struct Header
{
    size_t size{};
    Header* next{nullptr};
};

struct SearchResult
{
    Header* current{nullptr};
    Header* prev{nullptr};
};

struct FirstFitPolicy
{
    SearchResult find(Header* head, size_t requested_size)
    {
        Header* prev{nullptr};
        Header* curr{head};

        while (curr != nullptr)
        {
            if (curr->size >= requested_size)
                return {.current = curr, .prev = prev};

            prev = curr;
            curr = curr->next;
        }

        return {.current = nullptr, .prev = nullptr};
    }

    void reset()
    {
    }
};

struct BestFitPolicy
{
    SearchResult find(Header* head, size_t requested_size)
    {
        SearchResult result{.current = nullptr, .prev = nullptr};

        Header* prev{nullptr};
        Header* curr{head};

        while (curr != nullptr)
        {
            if (curr->size == requested_size)
                return {.current = curr, .prev = prev};
            else if (curr->size > requested_size)
            {
                if (result.current == nullptr || result.current->size > curr->size)
                {
                    result = {.current = curr, .prev = prev};
                }
            }

            prev = curr;
            curr = curr->next;
        }

        return result;
    }

    void reset()
    {
    }
};

struct WorstFitPolicy
{
    SearchResult find(Header* head, size_t requested_size)
    {
        SearchResult result{.current = nullptr, .prev = nullptr};

        Header* prev{nullptr};
        Header* curr{head};

        while (curr != nullptr)
        {
            if (curr->size > requested_size)
            {
                if (result.current == nullptr || result.current->size < curr->size)
                {
                    result = {.current = curr, .prev = prev};
                }
            }

            prev = curr;
            curr = curr->next;
        }

        return result;
    }

    void reset()
    {
    }
};

template <typename Policy>
concept SearchPolicy = requires(Policy p, Header* head, size_t size) {
                           { p.find(head, size) } -> std::same_as<SearchResult>;
                           { p.reset() } -> std::same_as<void>;
                       };

template <size_t Capacity, typename SearchPolicy = FirstFitPolicy>
class Allocator
{
public:
    explicit Allocator()
    {
        void* ptr = free_space_.data();
        size_t size = free_space_.size();
        void* aligned_ptr = std::align(alignof(Header), sizeof(Header), ptr, size);

        if (aligned_ptr == nullptr)
            return;

        head_ =
            std::construct_at(static_cast<Header*>(aligned_ptr), size - sizeof(Header), nullptr);
    }

    void* malloc(size_t size)
    {
        if (size == 0)
            return nullptr;

        // 1. Align requested size
        constexpr size_t Alignment = alignof(std::max_align_t);
        size_t aligned_size = (size + Alignment - 1) & ~(Alignment - 1);

        // 2. Find candidate block
        SearchResult result = search_policy_.find(head_, aligned_size);
        if (result.current == nullptr)
            return nullptr;

        Header* curr = result.current;
        Header* prev = result.prev;

        // Minimum payload required for a valid remainder block (e.g., 8 bytes)
        constexpr size_t MinPayload = sizeof(Header);
        size_t needed_for_split = sizeof(Header) + MinPayload;

        // 3. Decide whether to split
        if (curr->size >= aligned_size + needed_for_split)
        {
            curr->size -= (sizeof(Header) + aligned_size);
            std::byte* new_header_addr = reinterpret_cast<std::byte*>(curr + 1) + curr->size;
            Header* alloc_header = std::construct_at(reinterpret_cast<Header*>(new_header_addr),
                                                     Header{.size = aligned_size, .next = nullptr});
            return static_cast<void*>(alloc_header + 1);
        }
        else
        {
            if (prev == nullptr)
                head_ = curr->next;
            else
                prev->next = curr->next;

            curr->next = nullptr;
            return static_cast<void*>(curr + 1);
        }
    }

    void free(void* ptr)
    {
        if (ptr == nullptr)
            return;

        Header* block = static_cast<Header*>(ptr) - 1;

        // Locate insertion point in free list
        Header* prev = nullptr;
        Header* curr = head_;

        while (curr != nullptr && curr < block)
        {
            prev = curr;
            curr = curr->next;
        }

        // 1. Try to coalesce with right neighbor (curr)
        if (curr != nullptr && reinterpret_cast<std::byte*>(block + 1) + block->size ==
                                   reinterpret_cast<std::byte*>(curr))
        {
            block->size += sizeof(Header) + curr->size;
            block->next = curr->next;
        }
        else
        {
            block->next = curr;
        }

        // 2. Try to coalesce with left neighbor (prev)
        if (prev != nullptr && reinterpret_cast<std::byte*>(prev + 1) + prev->size ==
                                   reinterpret_cast<std::byte*>(block))
        {
            prev->size += sizeof(Header) + block->size;
            prev->next = block->next;
        }
        else
        {
            if (prev == nullptr)
                head_ = block;
            else
                prev->next = block;
        }

        search_policy_.reset();
    }

private:
    std::array<std::byte, Capacity> free_space_;
    Header* head_{nullptr};
    SearchPolicy search_policy_{};
};
