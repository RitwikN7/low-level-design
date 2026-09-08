#include <cstddef>

template <typename T>
class MyVector
{
public:
    // copy constructor
    MyVector(const MyVector<T>& other)
        : size_(other.size_),
          capacity_(other.capacity_)
    {

        if (capacity_ <= 0)
        {
            array_ = nullptr;
            return;
        }

        array_ = new T[capacity_];

        for (size_t i = 0; i < size_; i++)
            array_[i] = other.array_[i];
    }

    // copy assignment operator
    MyVector<T>& operator=(const MyVector<T>& other)
    {
        if (this != &other)
        {
            size_ = other.size_;
            capacity_ = other.capacity_;

            delete[] array_;

            if (capacity_ > 0)
            {
                array_ = new T[capacity_];

                for (size_t i = 0; i < size_; i++)
                    array_[i] = other.array_[i];
            }
            else
                array_ = nullptr;
        }

        return *this;
    }

    // move constructor
    MyVector(MyVector<T>&& other) noexcept
        : size_(other.size_),
          capacity_(other.capacity_),
          array_(other.array_)
    {

        other.size_ = 0;
        other.capacity_ = 0;
        other.array_ = nullptr;
    }

    // move assignment operator
    MyVector<T>& operator=(MyVector<T>&& other) noexcept
    {
        if (this != &other)
        {
            size_ = other.size_;
            capacity_ = other.capacity_;

            delete[] array_;
            array_ = other.array_;

            other.size_ = 0;
            other.capacity_ = 0;
            other.array_ = nullptr;
        }

        return *this;
    }

    size_t getSize() const
    {
        return size_;
    }

    size_t getCapacity() const
    {
        return capacity_;
    }

    void pushBack(const T& val)
    {
        if (size_ < capacity_)
        {
            array_[size_++] = val;
            return;
        }

        size_t newCapacity = capacity_ == 0 ? 1 : capacity_ * 2;
        T* tmp = new T[newCapacity];
        for (size_t i = 0; i < size_; i++)
            tmp[i] = array_[i];

        delete[] array_;
        array_ = tmp;
        array_[size_++] = val;
        capacity_ = newCapacity;
    }

    T& operator[](size_t index)
    {
        return array_[index];
    }

    const T& operator[](size_t index) const
    {
        return array_[index];
    }

    ~MyVector()
    {
        delete[] array_;
    }

private:
    size_t size_{};
    size_t capacity_{};
    T* array_{nullptr};
};