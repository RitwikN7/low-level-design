#include <atomic>
#include <cstddef>
#include <utility>

struct ControlBlock
{
    std::atomic<size_t> refCount_{1};
};

template <typename T>
class MySharedPtr
{
public:
    constexpr MySharedPtr() noexcept = default;

    constexpr MySharedPtr(std::nullptr_t) noexcept
    {
    }

    constexpr explicit MySharedPtr(T* ptr)
    {
        try
        {
            if (ptr)
                cb_ = new ControlBlock();
        }
        catch (...)
        {
            delete ptr;
            throw;
        }
    }

    // copy constructor
    MySharedPtr(const MySharedPtr& other) noexcept
        : ptr_(other.ptr_),
          cb_(other.cb_)
    {
        addRef();
    }

    // copy assignment operator
    MySharedPtr& operator=(const MySharedPtr& other) noexcept
    {
        if (this != &other)
        {
            MySharedPtr temp(other);
            std::swap(this->ptr_, temp.ptr_);
            std::swap(this->cb_, temp.cb_);
        }

        return *this;
    }

    // move constructor
    MySharedPtr(MySharedPtr&& other) noexcept
        : ptr_(other.ptr_),
          cb_(other.cb_)
    {
    }

    // move assignment operator
    MySharedPtr& operator=(MySharedPtr&& other) noexcept
    {
        std::swap(this->ptr_, other.ptr_);
        std::swap(this->cb_, other.cb_);
        return *this;
    }

    // destructor
    ~MySharedPtr() noexcept
    {
        release();
    }

    T* get() const noexcept
    {
        return ptr_;
    }

    T& operator*() const noexcept
    {
        return *ptr_;
    }

    T* operator->() const noexcept
    {
        return ptr_;
    }

private:
    void addRef()
    {
        if (cb_ != nullptr)
            cb_->refCount_.fetch_add(1, std::memory_order_relaxed);
    }

    void release() noexcept
    {
        if (cb_ != nullptr)
        {
            if (cb_->refCount_.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete ptr_;
                delete cb_;
            }

            ptr_ = nullptr;
            cb_ = nullptr;
        }
    }

    T* ptr_{nullptr};
    ControlBlock* cb_{nullptr};
};
