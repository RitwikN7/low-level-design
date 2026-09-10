#include <cstddef>
#include <memory>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class MyVector
{
public:
    MyVector() = default;

    explicit MyVector(size_t capacity)
    {
        reserve(capacity);
    }

    // Copy Constructor
    MyVector(const MyVector& other)
    {
        if (other.capacity_ == 0)
            return;

        array_ = alloc_.allocate(other.capacity_);
        capacity_ = other.capacity_;

        size_t constructed = 0;
        try
        {
            for (; constructed < other.size_; ++constructed)
            {
                // Placement new: construct copy in uninitialized memory
                ::new (static_cast<void*>(array_ + constructed)) T(other.array_[constructed]);
            }
            size_ = other.size_;
        }
        catch (...)
        {
            // Roll back any constructed objects on exception
            for (size_t j = 0; j < constructed; ++j)
            {
                array_[j].~T();
            }
            alloc_.deallocate(array_, capacity_);
            throw;
        }
    }

    // Move Constructor
    MyVector(MyVector&& other) noexcept
        : array_(std::exchange(other.array_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0))
    {
    }

    // Destructor
    ~MyVector()
    {
        clear_and_deallocate();
    }

    // Copy Assignment (Copy-and-Swap idiom provides strong exception guarantee)
    MyVector& operator=(const MyVector& other)
    {
        if (this != &other)
        {
            MyVector temp(other);
            swap(temp);
        }
        return *this;
    }

    // Move Assignment
    MyVector& operator=(MyVector&& other) noexcept
    {
        if (this != &other)
        {
            clear_and_deallocate();
            array_ = std::exchange(other.array_, nullptr);
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }

    void swap(MyVector& other) noexcept
    {
        std::swap(array_, other.array_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Capacity management
    void reserve(size_t new_cap)
    {
        if (new_cap <= capacity_)
            return;

        T* new_array = alloc_.allocate(new_cap);
        size_t constructed = 0;

        try
        {
            for (; constructed < size_; ++constructed)
            {
                // std::move_if_noexcept preserves the strong exception guarantee:
                // moves if noexcept, falls back to copy if moving could throw.
                ::new (static_cast<void*>(new_array + constructed))
                    T(std::move_if_noexcept(array_[constructed]));
            }
        }
        catch (...)
        {
            for (size_t j = 0; j < constructed; ++j)
            {
                new_array[j].~T();
            }
            alloc_.deallocate(new_array, new_cap);
            throw;
        }

        // Destroy old elements and free old storage
        for (size_t i = 0; i < size_; ++i)
        {
            array_[i].~T();
        }
        if (array_)
        {
            alloc_.deallocate(array_, capacity_);
        }

        array_ = new_array;
        capacity_ = new_cap;
    }

    // In-place construction
    template <typename... Args>
    T& emplace_back(Args&&... args)
    {
        if (size_ == capacity_)
        {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }

        T* target = array_ + size_;
        ::new (static_cast<void*>(target)) T(std::forward<Args>(args)...);
        ++size_;
        return *target;
    }

    void push_back(const T& val)
    {
        emplace_back(val);
    }

    void push_back(T&& val)
    {
        emplace_back(std::move(val));
    }

    void pop_back()
    {
        if (size_ > 0)
        {
            --size_;
            array_[size_].~T(); // Explicit destructor call
        }
    }

    // Element Access & Observers
    T& operator[](size_t index)
    {
        return array_[index];
    }

    const T& operator[](size_t index) const
    {
        return array_[index];
    }

    size_t size() const noexcept
    {
        return size_;
    }

    size_t capacity() const noexcept
    {
        return capacity_;
    }

    bool empty() const noexcept
    {
        return size_ == 0;
    }

    T* data() noexcept
    {
        return array_;
    }

    const T* data() const noexcept
    {
        return array_;
    }

    // Iterators (enables range-for loops: `for (auto& x : vec)`)
    T* begin() noexcept
    {
        return array_;
    }

    T* end() noexcept
    {
        return array_ + size_;
    }

    const T* begin() const noexcept
    {
        return array_;
    }

    const T* end() const noexcept
    {
        return array_ + size_;
    }

private:
    void clear_and_deallocate() noexcept
    {
        if (array_ == nullptr)
            return;

        for (size_t i = 0; i < size_; ++i)
        {
            array_[i].~T();
        }

        alloc_.deallocate(array_, capacity_);
        array_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    Alloc alloc_;
    T* array_{nullptr};
    size_t size_{0};
    size_t capacity_{0};
};
