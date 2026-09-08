#include <cstddef>
#include <functional>
#include <list>
#include <vector>

template <typename K, typename V>
class MyHashMap
{
public:
    MyHashMap() = default;

    // copy constructor
    MyHashMap(const MyHashMap<K, V>& other)
        : data_(other.data_),
          max_load_factor(other.max_load_factor),
          size_(other.size_),
          bucket_count_(other.bucket_count_)
    {
    }

    ~MyHashMap() = default;

    // copy assignment operator
    MyHashMap<K, V>& operator=(const MyHashMap<K, V>& other)
    {
        if (this != &other)
        {
            size_ = other.size_;
            bucket_count_ = other.bucket_count_;
            max_load_factor = other.max_load_factor;
            data_ = other.data_;
        }

        return *this;
    }

    // move constructor
    MyHashMap(MyHashMap<K, V>&& other) noexcept
        : data_(std::move(other.data_)),
          max_load_factor(other.max_load_factor),
          size_(other.size_),
          bucket_count_(other.bucket_count_)
    {
    }

    // move assignment operator
    MyHashMap<K, V>& operator=(MyHashMap<K, V>&& other) noexcept
    {
        if (this != &other)
        {
            size_ = other.size_;
            bucket_count_ = other.bucket_count_;
            max_load_factor = other.max_load_factor;
            data_ = std::move(other.data_);
        }

        return *this;
    }

    void insert(const K& key, const V& val)
    {
        size_t index = getIndex(key);
        auto& cell = data_[index];
        for (auto& node : cell)
        {
            if (node.key_ == key)
            {
                node.val_ = val;
                return;
            }
        }

        cell.push_back(HashNode(key, val));
        size_++;

        double load_factor = static_cast<double>(size_) / bucket_count_;
        if (load_factor > max_load_factor)
            rehash();
    }

    bool remove(const K& key)
    {
        size_t index = getIndex(key);
        auto& cell = data_[index];
        for (auto iter = cell.begin(); iter != cell.end(); ++iter)
        {
            if (iter->key_ == key)
            {
                data_[index].erase(iter);
                size_--;
                return true;
            }
        }

        return false;
    }

    bool get(const K& key, V& val) const
    {
        size_t index = getIndex(key);
        const auto& cell = data_[index];
        for (const auto& node : cell)
        {
            if (node.key_ == key)
            {
                val = node.val_;
                return true;
            }
        }

        return false;
    }

    size_t getSize() const
    {
        return size_;
    }

private:
    struct HashNode
    {
        K key_;
        V val_;

        HashNode(const K& key, const V& val)
            : key_(key),
              val_(val)
        {
        }
    };

    size_t getIndex(const K& key) const
    {
        std::hash<K> hasher{};
        return hasher(key) % bucket_count_;
    }

    void rehash()
    {
        bucket_count_ = (bucket_count_ * 2) + 1;
        std::vector<std::list<HashNode>> tmp{bucket_count_};

        for (auto& cell : data_)
        {
            for (auto& node : cell)
            {
                size_t newIndex = getIndex(node.key_);
                tmp[newIndex].push_back(std::move(node));
            }
        }

        data_ = std::move(tmp);
    }

    std::vector<std::list<HashNode>> data_{11};
    double max_load_factor{0.75};
    size_t size_{};
    size_t bucket_count_{11};
};