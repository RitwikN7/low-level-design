#include <cstddef>
#include <vector>

class SegmentTree
{
public:
    SegmentTree(const std::vector<int>& nums)
        : segments_(4 * nums.size(), 0),
          size_(nums.size())
    {
        build(nums, 1, 0, size_ - 1);
    }

    int sum(size_t left, size_t right)
    {
        return sumHelper(left, right, 1, 0, size_ - 1);
    }

    void update(size_t index, int value)
    {
        updateHelper(index, value, 1, 0, size_ - 1);
    }

private:
    void build(const std::vector<int>& nums, size_t node, size_t tree_left, size_t tree_right)
    {
        if (tree_left == tree_right)
        {
            segments_[node] = nums[tree_left];
            return;
        }

        size_t mid = tree_left + ((tree_right - tree_left) / 2);
        build(nums, node * 2, tree_left, mid);
        build(nums, (node * 2) + 1, mid + 1, tree_right);
        segments_[node] = segments_[node * 2] + segments_[(node * 2) + 1];
    }

    int sumHelper(size_t left, size_t right, size_t node, size_t tree_left, size_t tree_right)
    {
        if (left <= tree_left && tree_right <= right)
            return segments_[node];

        if (tree_right < left || tree_left > right)
            return 0;

        size_t mid = tree_left + ((tree_right - tree_left) / 2);
        int left_sum = sumHelper(left, right, node * 2, tree_left, mid);
        int right_sum = sumHelper(left, right, (node * 2) + 1, mid + 1, tree_right);
        return left_sum + right_sum;
    }

    void updateHelper(size_t index, int value, size_t node, size_t tree_left, size_t tree_right)
    {
        if (tree_left == tree_right)
        {
            segments_[node] = value;
            return;
        }

        size_t mid = tree_left + ((tree_right - tree_left) / 2);
        if (index <= mid)
        {
            updateHelper(index, value, node * 2, tree_left, mid);
        }
        else
        {
            updateHelper(index, value, (node * 2) + 1, mid + 1, tree_right);
        }

        segments_[node] = segments_[node * 2] + segments_[(node * 2) + 1];
    }

    std::vector<int> segments_;
    size_t size_;
};