#include <bits/stdc++.h>

std::vector<std::pair<int, int>> maximum_matching(
    int n,
    const std::vector<std::pair<int, int>> &edg) {
    struct DSU {
        std::vector<int> prv;
        DSU(int n = 0) : prv(n, -1) { ; }
        void clear(int n) { prv.assign(n, -1); }
        int get(int i) { return prv[i] == -1 ? i : prv[i] = get(prv[i]); }
    };

    std::vector<int> vec(n, -1);
    std::vector<std::vector<int>> gr(n);
    {
        std::vector<int> cnt(n);
        for (auto [u, v] : edg) {
            cnt[u]++, cnt[v]++;
        }
        for (int i = 0; i < n; i++) {
            gr[i].reserve(cnt[i]);
        }
        for (auto [u, v] : edg) {
            gr[u].push_back(v);
            gr[v].push_back(u);
        }
    }

    DSU dsu(n), dsu2(n + 1);
    std::vector<std::array<int, 2>> prv(n, {-1, -1});
    std::vector<std::array<int, 3>> fwd(n, {-1, -1, -1}), bwd(n, {-1, -1, -1});
    std::vector<int> bl_dir(n, -1), bl_prv(n, -1), bl_lvl(n, -1);
    std::vector<int> bl_depth(n, -1), bl_jump(n, -1), bl_ord(n, -1);
    std::vector<int> depth(n, -1);
    std::vector<int64_t> used(n, -1);
    int64_t used_mark = -1;
    std::deque<int> deque;
    std::vector<std::pair<int, int>> aug_vec;
    int total_bl = 0;

    auto find_lca = [&](int u, int v) {
        used_mark++;
        u = dsu.get(u), v = dsu.get(v);
        while (u >= 0 || v >= 0) {
            if (u != -1) {
                if (used[u] == used_mark) {
                    return u;
                }
                used[u] = used_mark, (u = prv[u][0] >= 0 ? dsu.get(prv[u][0]) : prv[u][0]);
            }
            std::swap(u, v);
        }
        return -1;
    };

    auto contract_blossom = [&](int u0, int v0, int l) {
        bl_lvl[l] = ++total_bl;
        bl_ord.push_back(l);
        for (int ch = 0; ch < 2; ch++) {
            int x0 = ch == 0 ? u0 : v0;
            int y0 = ch == 0 ? v0 : u0;
            int x = dsu.get(x0);
            while (x != l) {
                int p0 = prv[x][0];
                int p = dsu.get(p0);

                if (depth[x] == 1) {
                    deque.push_back(x);
                }
                bl_dir[x] = depth[x] ^ ch;
                bl_prv[x] = l;
                fwd[x] = {dsu.get(p0), p0, prv[x][1]};
                bwd[x] = {dsu.get(y0), y0, x0};

                if (ch) {
                    std::swap(fwd[x], bwd[x]);
                }
                x0 = p0, y0 = prv[x][1], x = p;
            }
        }
        for (auto x : std::array<int, 2>{dsu.get(u0), dsu.get(v0)}) {
            for (; x != l; x = dsu.get(prv[x][0])) {
                dsu.prv[x] = l;
            }
        }
    };
    auto init_jump = [&]() {
        for (int i = bl_ord.size() - 1; i >= 0; i--) {
            int v = bl_ord[i];
            if (bl_depth[v] == -1) {
                int f = bl_prv[v];
                bl_depth[v] = bl_depth[f] + 1;
                bl_jump[v] = f;
                if (bl_depth[v] > 1 && bl_depth[f] - bl_depth[bl_jump[f]] ==
                                           bl_depth[bl_jump[f]] - bl_depth[bl_jump[bl_jump[f]]]) {
                    bl_jump[v] = bl_jump[bl_jump[f]];
                }
            }
        }
    };
    auto jump = [&](int x, int d) {
        while (bl_lvl[bl_prv[x]] < d) {
            if (bl_lvl[bl_jump[x]] < d) {
                x = bl_jump[x];
            } else {
                x = bl_prv[x];
            }
        }
        return x;
    };
    auto augment = [&](int u0, int v0) {
        int u = dsu.get(u0), v = dsu.get(v0);
        std::list<std::pair<int, int>> list;
        for (int x = dsu.get(u0); prv[x][0] != -1; x = dsu.get(prv[x][0])) {
            list.push_front({prv[x][0], prv[x][1]});
        }
        list.push_front({-1, prv[u][0] == -1 ? u : dsu.get(list.front().first)});
        list.push_back({u0, v0});
        for (int x = dsu.get(v0); prv[x][0] != -1; x = dsu.get(prv[x][0])) {
            list.push_back({prv[x][1], prv[x][0]});
        }
        list.push_back({prv[v][0] == -1 ? v : dsu.get(list.back().second), -1});

        for (auto it = std::next(list.begin()); it != list.end();) {
            auto [a1, a2] = *std::prev(it);
            auto [b1, b2] = *it;
            if (a2 == b1) {
                it++;
                continue;
            }
            bool f1 = (a1 == -1 || vec[a1] == a2);
            bool f2 = (b2 == -1 || vec[b1] == b2);
            assert(f1 != f2);
            bool rev = false;
            if (f2) {
                rev = true;
                std::swap(a1, b2);
                std::swap(a2, b1);
            }
            decltype(list) list2;
            int c = jump(b1, bl_lvl[a2]);
            auto &mv = !bl_dir[c] ? fwd : bwd;
            for (int x = c; x != a2; x = mv[x][0]) {
                list2.push_front({mv[x][1], mv[x][2]});
            }
            if (!rev) {
                it = list.insert(it, list2.begin(), list2.end());
            } else {
                for (auto &[a, b] : list2) {
                    std::swap(a, b);
                }
                it = list.insert(it, list2.rbegin(), list2.rend());
            }
        }
        list.pop_front(), list.pop_back();
        assert(list.size() % 2 == 1);
        for (auto it = list.begin();; it = std::next(it, 2)) {
            vec[it->first] = it->second;
            vec[it->second] = it->first;
            if (std::next(it) == list.end()) {
                break;
            }
        }
    };

    auto process_edge = [&](int u0, int v0) {
        int u = dsu.get(u0);
        int v = dsu.get(v0);
        if (u == v || depth[v] == 1 || dsu2.get(u) == n || dsu2.get(v) == n) {
            return;
        }
        if (depth[v] == -1) {
            int w = vec[v];
            dsu2.prv[v] = u;
            dsu2.prv[w] = u;
            depth[v] = 1;
            depth[w] = 0;
            prv[v] = {u0, v0};
            prv[w] = {v0, w};
            deque.push_back(w);
        } else {
            int l = find_lca(u, v);
            if (l != -1) {
                contract_blossom(u0, v0, l);
            } else {
                // augment(u0, v0);
                aug_vec.push_back({u0, v0});
                for (int x : std::array<int, 2>{u, v}) {
                    for (; x != -1; x = (prv[x][0] == -1 ? -1 : dsu.get(prv[x][0]))) {
                        dsu2.prv[x] = n;
                    }
                }
            }
        }
    };

    while (true) {
        dsu.clear(n), dsu2.clear(n + 1);
        prv.assign(n, {-1, -1});
        fwd.assign(n, {-1, -1, -1}), bwd.assign(n, {-1, -1, -1});
        bl_dir.assign(n, -1), bl_prv.assign(n, -1), bl_lvl.assign(n, -1);
        depth.assign(n, -1);
        deque.clear();
        aug_vec.clear();
        bl_ord.assign(n, -1);
        bl_depth.assign(n, -1);
        bl_jump.assign(n, -1);
        std::iota(bl_ord.begin(), bl_ord.end(), 0);
        std::iota(bl_prv.begin(), bl_prv.end(), 0);
        total_bl = 0;

        for (int i = 0; i < n; i++) {
            if (vec[i] == -1) {
                depth[i] = 0;
                bl_lvl[i] = total_bl;
                deque.push_back(i);
            }
        }
        while (deque.size()) {
            int u0 = deque.front();
            deque.pop_front();
            for (int v0 : gr[u0]) {
                process_edge(u0, v0);
            }
        }

        if (aug_vec.size()) {
            init_jump();
            std::cerr << aug_vec.size() << " ";
            for (auto [u0, v0] : aug_vec) {
                augment(u0, v0);
            }

        } else {
            break;
        }
    }

    std::vector<std::pair<int, int>> res;
    for (int i = 0; i < n; i++) {
        if (vec[i] > i) {
            res.push_back({i, vec[i]});
        }
    }
    return res;
}

int32_t main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, m;
    std::cin >> n >> m;
    std::vector<std::pair<int, int>> edg(m);
    for (auto &[u, v] : edg) {
        std::cin >> u >> v;
    }
    std::mt19937 rnd;
    std::shuffle(edg.begin(), edg.end(), rnd);

    clock_t beg = clock();
    auto ans = maximum_matching(n, edg);
    std::cerr << "work: " << (clock() - beg) * 1.0 / CLOCKS_PER_SEC * 1000 << "ms" << std::endl;

    std::cout << (int)ans.size() << '\n';
    for (auto [u, v] : ans) {
        std::cout << u << ' ' << v << '\n';
    }

    return 0;
}
