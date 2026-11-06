#include <bits/stdc++.h>

struct LinkCut {
    struct Node {
        int next[2], prev, sz, flip;
        Node() : next{-1, -1}, prev(-1), sz(1), flip(0) { ; }
    };

    std::vector<Node> data;

    LinkCut(int n = 0) : data(n) { ; }

    int get_sz(int i) { return i == -1 ? 0 : data[i].sz; }
    void set_prev(int i, int p) { i != -1 ? data[i].prev = p : 0; }
    void flip(int i) { i != -1 ? data[i].flip ^= 1 : 0; }
    void pull(int i) { data[i].sz = get_sz(data[i].next[0]) + 1 + get_sz(data[i].next[1]); }
    void push(int i) {
        data[i].flip ? (flip(data[i].next[0]), flip(data[i].next[1]),
                        std::swap(data[i].next[0], data[i].next[1]), data[i].flip = false)
                     : 0;
    }
    int ch_num(int i) {
        return data[i].prev == -1
                   ? -1
                   : (data[data[i].prev].next[0] == i ? 0 : (data[data[i].prev].next[1] == i ? 1 : -1));
    }

    void rotate(int i) {
        int j = data[i].prev, k = data[j].prev, ni = ch_num(i), nj = ch_num(j);
        data[j].next[ni] = data[i].next[!ni], set_prev(data[i].next[!ni], j);
        data[i].next[!ni] = j, data[i].prev = k, data[j].prev = i, pull(j), pull(i);
        if (nj != -1) data[k].next[nj] = i;
    }

    void splay(int i) {
        push(i);
        while (ch_num(i) != -1) {
            int j = data[i].prev, k = data[j].prev;
            if (ch_num(j) == -1) {
                push(j), push(i), rotate(i);
            } else {
                push(k), push(j), push(i), rotate(ch_num(i) == ch_num(j) ? j : i), rotate(i);
            }
        }
    }

    void expose(int i) {
        splay(i), data[i].next[1] = -1, pull(i);
        while (data[i].prev != -1) {
            splay(data[i].prev), data[data[i].prev].next[1] = i, pull(data[i].prev), splay(i);
        }
    }

    void ch_root(int i) { expose(i), flip(i); }
    void link(int i, int j) { ch_root(j), data[j].prev = i; }
    void cut(int i, int j) { ch_root(i), expose(i), splay(j), data[j].prev = -1; }
    int get_dist(int i, int j) {
        return i == j ? 0 : (ch_root(i), expose(j), (data[i].prev != -1 ? data[j].sz - 1 : -1));
    }
};
