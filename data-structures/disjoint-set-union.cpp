#include <utility>
#include <vector>

class DSU
{
public:
    explicit DSU(int n)
        : parent_(n, 0),
          size_(n, 0)
    // rank_(n, 0)
    {
        for (int i{}; i < n; ++i)
        {
            parent_[i] = i;
            size_[i] = 1;
            // rank_[i] = 0;
        }
    }

    int find(int v)
    {
        if (parent_[v] == v)
            return v;

        return parent_[v] = find(parent_[v]);
    }

    void union_by_size(int u, int v)
    {
        int a = find(u);
        int b = find(v);

        if (a == b)
            return;

        if (size_[b] > size_[a])
            std::swap(a, b);

        parent_[b] = a;
        size_[a] += size_[b];
    }

    /*
    void union_by_rank(int u, int v)
    {
        int a = find(u);
        int b = find(v);

        if (a == b)
            return;

        if (rank[a] < rank[b])
            parent_[a] = b;
        else if (rank[a] > rank[b])
            parent_[b] = a;
        else
        {
            parent[b] = a;
            rank_[a]++;
        }
    }
    */

    bool is_connected(int u, int v)
    {
        return find(u) == find(v);
    }

private:
    std::vector<int> parent_;
    std::vector<int> size_;
    // std::vector<int> rank_;
};
