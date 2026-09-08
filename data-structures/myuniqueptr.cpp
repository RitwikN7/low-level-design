#include <cstddef>
#include <utility>

template <typename T>
class MyUniquePtr
{
public:
    constexpr MyUniquePtr() noexcept = default;

    constexpr MyUniquePtr(std::nullptr_t) noexcept
    {
    }

    constexpr explicit MyUniquePtr(T* ptr) noexcept
        : ptr_(ptr)
    {
    }

    // copy constructor
    MyUniquePtr(const MyUniquePtr&) = delete;

    // copy assignment operator
    MyUniquePtr& operator=(const MyUniquePtr&) = delete;

    // move constructor
    constexpr MyUniquePtr(MyUniquePtr&& other) noexcept
    {
        std::swap(this->ptr_, other.ptr_);
    }

    // move assignment operator
    constexpr MyUniquePtr& operator=(MyUniquePtr&& other) noexcept
    {
        if (this != &other)
            std::swap(this->ptr_, other.ptr_);

        return *this;
    }

    // destructor
    constexpr ~MyUniquePtr() noexcept
    {
        delete ptr_;
    }

    constexpr T* get() const noexcept
    {
        return ptr_;
    }

    constexpr T* operator->() const noexcept
    {
        return ptr_;
    }

    constexpr T& operator*() const noexcept
    {
        return *ptr_;
    }

    constexpr explicit operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    constexpr T* release() noexcept
    {
        return std::exchange(this->ptr_, nullptr);
    }

    constexpr void reset(T* ptr = nullptr) noexcept
    {
        delete ptr_;
        this->ptr_ = ptr;
    }

private:
    T* ptr_{nullptr};
};