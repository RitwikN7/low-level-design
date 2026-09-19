#include <cstddef>
#include <span>

size_t binary_search(std::span<const int> data, int target)
{
    size_t left{};
    size_t right{data.size()};

    while (left < right)
    {
        size_t mid = left + ((right - left) / 2);
        const auto& elem = data[mid];
        if (elem == target)
            return mid;
        else if (elem < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid;
        }
    }

    return data.size();
}

size_t lower_bound(std::span<const int> data, int target)
{
    size_t left{};
    size_t right{data.size()};

    while (left < right)
    {
        size_t mid = left + ((right - left) / 2);
        const auto& elem = data[mid];

        if (elem >= target)
        {
            right = mid;
        }
        else
        {
            left = mid + 1;
        }
    }

    return left;
}

size_t upper_bound(std::span<const int> data, int target)
{
    size_t left{};
    size_t right{data.size()};

    while (left < right)
    {
        size_t mid = left + ((right - left) / 2);
        const auto& elem = data[mid];

        if (elem > target)
        {
            right = mid;
        }
        else
        {
            left = mid + 1;
        }
    }

    return left;
}
