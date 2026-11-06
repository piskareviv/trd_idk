/* input: undirected graph */ /* output: tree in form [p_i, w_i], res[0] is empty */ /* runs
    dinic n-1 times */ /* mincut in this tree <=> mincut in original graph */ /* g - (u, v, w)
    */
vector<pair<int, int>> gomory_hu(int n, vector<tuple<int, int, int>> const &g) { Dinic f(n, 0,
    0); for (auto [v, u, w] : g) f.add(v, u, w, w); vector<pair<int, int>> res(n); vector<int>
    pr(n, 0); for (int i = 1; i < n; i++) { for (auto &j : f.ed) j.f = 0; f.st = i; f.fin =
    pr[i]; auto flow = f.get(); vector<bool> cut(n); for (int j = 0; j < n; j++) cut[j] =
    (f.ds[j] != -1); for (int j = i + 1; j < n; j++) { if (cut[j] == cut[i] && pr[j] == pr[i])
    pr[j] = i; } res[i] = {pr[i], flow}; } return res; }
