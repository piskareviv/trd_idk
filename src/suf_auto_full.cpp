#include <bits/stdc++.h>

struct Node {
    int32_t go[26];
    int32_t suf_link = -1;
    int32_t parent = -1;
    int32_t count = 1;

    int32_t min_len = 1'000'000'000, max_len = -1'000'000'000;

    Node() {
        for (int32_t i = 0; i < 26; i++)
            go[i] = -1;
    }

    void copy_go(Node& node) {
        for (int32_t i = 0; i < 26; i++)
            go[i] = node.go[i];
    }
};

int32_t next = 1;
int32_t extend(int32_t last, int32_t ch, Node* nodes) {
    int32_t b = next++;
    nodes[b].parent = last;
    for (int32_t a = last; a > -1; a = nodes[a].suf_link) {
        if (nodes[a].go[ch] == -1) {
            nodes[a].go[ch] = b;
            continue;
        }

        int32_t c = nodes[a].go[ch];
        if (nodes[c].parent == a) {
            nodes[b].suf_link = c;
            return b;
        }

        int32_t clone = next++;
        nodes[clone].copy_go(nodes[c]);
        nodes[clone].parent = a;

        nodes[clone].suf_link = nodes[c].suf_link;
        nodes[c].suf_link = clone;
        nodes[b].suf_link = clone;

        nodes[clone].count = 0;

        for (; a > -1; a = nodes[a].suf_link)
            if (nodes[a].go[ch] == c)
                nodes[a].go[ch] = clone;
            else
                break;
        return b;
    }
    nodes[b].suf_link = 0;
    return b;
}

void dfs_topsort(int32_t v, bool* vis, Node* nodes, std::vector<int32_t>& topsort) {
    vis[v] = true;
    for (int32_t i = 0; i < 26; i++)
        if (nodes[v].go[i] != -1 && !vis[nodes[v].go[i]])
            dfs_topsort(nodes[v].go[i], vis, nodes, topsort);
    topsort.push_back(v);
}

void dfs_topsort2(int32_t v, bool* vis, Node* nodes, std::vector<int32_t>& topsort) {
    vis[v] = true;
    for (int32_t i = 0; i < 26; i++)
        if (nodes[v].suf_link != -1 && !vis[nodes[v].suf_link])
            dfs_topsort2(nodes[v].suf_link, vis, nodes, topsort);
    topsort.push_back(v);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);

    std::vector<int32_t>* divisors = new std::vector<int32_t>[1'000'005];
    for (int32_t i = 1; i < 1'000'005; i++)
        for (int32_t j = i; j < 1'000'005; j += i)
            divisors[j].push_back(i);

    int32_t n;
    std::cin >> n;

    std::string str;
    std::cin >> str;

    Node* nodes = new Node[2 * n];
    nodes[0].count = 0;
    int32_t last = 0;
    for (int32_t i = 0; i < n; i++)
        last = extend(last, str[i] - 'a', nodes);

    std::vector<int32_t> topsort;
    bool* vis = new bool[2 * n];
    for (int32_t i = 0; i < 2 * n; i++)
        vis[i] = false;
    dfs_topsort(0, vis, nodes, topsort);

    std::vector<int32_t> topsort2;
    for (int32_t i = 0; i < 2 * n; i++)
        vis[i] = false;
    for (int32_t i = 0; i < next; i++)
        if (!vis[i])
            dfs_topsort2(i, vis, nodes, topsort2);
    for (int32_t i = topsort2.size() - 1; i >= 0; i--)
        if (nodes[topsort2[i]].suf_link != -1)
            nodes[nodes[topsort2[i]].suf_link].count += nodes[topsort2[i]].count;

    nodes[0].min_len = 0;
    nodes[0].max_len = 0;
    for (int32_t i = topsort.size() - 1; i >= 0; i--)
        for (int32_t j = 0; j < 26; j++)
            if (nodes[topsort[i]].go[j] != -1) {
                nodes[nodes[topsort[i]].go[j]].min_len = std::min(nodes[nodes[topsort[i]].go[j]].min_len, nodes[topsort[i]].min_len + 1);
                nodes[nodes[topsort[i]].go[j]].max_len = std::max(nodes[nodes[topsort[i]].go[j]].max_len, nodes[topsort[i]].max_len + 1);
            }

    int64_t ans = 0;
    for (int32_t i = 0; i < next; i++) {
        int32_t num_occ = nodes[i].count;
        for (int32_t j : divisors[num_occ])
            if (j >= nodes[i].min_len && j <= nodes[i].max_len)
                ans += num_occ;
    }

    std::cout << ans;
    return 0;
}
