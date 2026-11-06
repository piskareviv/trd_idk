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
