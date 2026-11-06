// src: https://judge.yosupo.jp/submission/233128

#include <bits/stdc++.h>
using namespace std;

struct dominator_tree {
    int n, t;
    vector<basic_string<int>> g, rg, bucket;
    vector<int> arr, par, rev, sdom, dom, dsu, label;

    dominator_tree(int n)
        : g(n),
          rg(n),
          bucket(n),
          arr(n, -1),
          par(n),
          rev(n),
          sdom(n),
          dom(n),
          dsu(n),
          label(n),
          n(n),
          t(0) {}

    void add_edge(int u, int v) { g[u] += v; }

    void dfs(int u) {
        arr[u] = t;
        rev[t] = u;
        label[t] = sdom[t] = dsu[t] = t;
        t++;
        for (int w : g[u]) {
            if (arr[w] == -1) {
                dfs(w);
                par[arr[w]] = arr[u];
            }
            rg[arr[w]] += arr[u];
        }
    }

    int find(int u, int x = 0) {
        if (u == dsu[u]) return x ? -1 : u;
        int v = find(dsu[u], x + 1);
        if (v < 0) return u;
        if (sdom[label[dsu[u]]] < sdom[label[u]]) label[u] = label[dsu[u]];
        dsu[u] = v;
        return x ? v : label[u];
    }

    vector<int> run(int root) {
        dfs(root);
        iota(dom.begin(), dom.end(), 0);
        for (int i = t - 1; i >= 0; i--) {
            for (int w : rg[i]) sdom[i] = min(sdom[i], sdom[find(w)]);
            if (i) bucket[sdom[i]] += i;
            for (int w : bucket[i]) {
                int v = find(w);
                if (sdom[v] == sdom[w])
                    dom[w] = sdom[w];
                else
                    dom[w] = v;
            }
            if (i > 1) dsu[i] = par[i];
        }
        for (int i = 1; i < t; i++) {
            if (dom[i] != sdom[i]) dom[i] = dom[dom[i]];
        }
        vector<int> outside_dom(n);
        iota(begin(outside_dom), end(outside_dom), 0);
        for (int i = 0; i < t; i++) outside_dom[rev[i]] = rev[dom[i]];
        return outside_dom;
    }
};

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    int n, m, r;
    cin >> n >> m >> r;
    dominator_tree d(n);
    while (m--) {
        int x, y;
        cin >> x >> y;
        d.add_edge(x, y);
    }
    auto v = d.run(r);
    for (int i = 0; i < n; i++) {
        if (i != r && v[i] == i)
            cout << "-1 ";
        else
            cout << v[i] << ' ';
    }
    return 0;
}
