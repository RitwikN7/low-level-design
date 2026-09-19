#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

// 1. Insertion sort
void insertion_sort(std::vector<int>& arr)
{
    size_t n{arr.size()};
    for (size_t i{1}; i < n; ++i)
    {
        int key = arr[i];
        size_t j = i - 1;
        while (j != std::numeric_limits<size_t>::max() && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            --j;
        }

        arr[j + 1] = key;
    }
}

// 2. Bubble sort
void bubble_sort(std::vector<int>& arr)
{
    size_t n{arr.size()};
    for (size_t i{}; i < n - 1; ++i)
    {
        bool swapped{};
        for (size_t j{}; j < n - i - 1; ++j)
        {
            if (arr[j] > arr[j + 1])
            {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        if (!swapped)
            break;
    }
}

// 3. Selection sort
void selection_sort(std::vector<int>& arr)
{
    size_t n{arr.size()};
    for (size_t i{}; i < n - 1; ++i)
    {
        size_t min_idx = i;
        for (size_t j{i + 1}; j < n; ++j)
        {
            if (arr[j] < arr[min_idx])
                min_idx = j;
        }

        if (min_idx != i)
            std::swap(arr[min_idx], arr[i]);
    }
}

// 4. Merge sort
void merge(std::vector<int>& arr, size_t left, size_t mid, size_t right)
{
    size_t n1 = mid - left + 1;
    size_t n2 = right - mid;

    std::vector<int> left_arr;
    left_arr.reserve(n1);
    std::vector<int> right_arr;
    right_arr.reserve(n2);

    for (size_t i{left}; i <= mid; ++i)
    {
        left_arr.push_back(arr[i]);
    }

    for (size_t i{mid + 1}; i < right; ++i)
    {
        left_arr.push_back(arr[i]);
    }

    size_t p1{};
    size_t p2{};
    size_t k{left};

    while (p1 < n1 && p2 < n2)
    {
        if (left_arr[p1] < right_arr[p2])
        {
            arr[k++] = left_arr[p1++];
        }
        else
        {
            arr[k++] = right_arr[p2++];
        }
    }

    while (p1 < n1)
    {
        arr[k++] = left_arr[p1++];
    }

    while (p2 < n2)
    {
        arr[k++] = right_arr[p2++];
    }
}

void merge_sort(std::vector<int>& arr, size_t left, size_t right)
{
    if (left >= right)
        return;

    size_t mid = left + ((right - left) / 2);

    merge_sort(arr, left, mid);
    merge_sort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// 5. Quick sort

// 6. Bucket sort

// 7. Counting sort
