#include <cstddef>
#include <functional>
#include <list>
#include <utility>
#include <vector>

template <typename T>
class MyHashSet
{
public:
    MyHashSet() = default;

    ~MyHashSet() = default;

    // copy constructor
    MyHashSet(MyHashSet<T>& other)
        : data_(other.data_),
          load_factor_(other.load_factor_),
          bucket_count_(other.bucket_count_),
          size_(other.size_)
    {
    }

    // copy assignment
    MyHashSet<T>& operator=(const MyHashSet<T>& other)
    {
        if (this != &other)
        {
            data_ = other.data_;
            load_factor_ = other.load_factor_;
            bucket_count_ = other.bucket_count_;
            size_ = other.size_;
        }

        return *this;
    }

    // move constructor
    MyHashSet(MyHashSet<T>&& other) noexcept
    {
        std::swap(data_, other.data_);
        std::swap(load_factor_, other.load_factor_);
        std::swap(bucket_count_, other.bucket_count_);
        std::swap(size_, other.size_);
    }

    // move assignment
    MyHashSet<T>& operator=(MyHashSet<T>&& other) noexcept
    {
        if (this != &other)
        {
            std::swap(data_, other.data_);
            std::swap(load_factor_, other.load_factor_);
            std::swap(bucket_count_, other.bucket_count_);
            std::swap(size_, other.size_);
        }

        return *this;
    }

    size_t size() const
    {
        return size_;
    }

    bool insert(const T& elem)
    {
        size_t index = getIndex(elem);
        auto& bucket = data_[index];
        for (const auto& node : bucket)
        {
            if (node == elem)
                return false;
        }

        bucket.push_back(elem);
        size_++;

        if (static_cast<double>(size_) / bucket_count_ > load_factor_)
            rehash();

        return true;
    }

    bool erase(const T& elem)
    {
        size_t index = getIndex(elem);
        auto& bucket = data_[index];
        for (auto iter = bucket.begin(); iter != bucket.end(); ++iter)
        {
            if (*iter == elem)
            {
                bucket.erase(iter);
                size_--;
                return true;
            }
        }

        return false;
    }

    bool exists(const T& elem) const
    {
        size_t index = getIndex(elem);
        auto& bucket = data_[index];
        for (const auto& node : bucket)
        {
            if (node == elem)
                return true;
        }

        return false;
    }

private:
    size_t getIndex(const T& elem) const
    {
        std::hash<T> hasher{};
        return hasher(elem) % bucket_count_;
    }

    void rehash()
    {
        bucket_count_ = (bucket_count_ * 2) + 1;
        std::vector<std::list<T>> tmp(bucket_count_);
        for (const auto& bucket : data_)
        {
            for (const auto& node : bucket)
            {
                size_t index = getIndex(node);
                tmp[index].push_back(node);
            }
        }

        data_ = std::move(tmp);
    }

    std::vector<std::list<T>> data_{11};
    double load_factor_{0.75};
    size_t bucket_count_{11};
    size_t size_{};
};