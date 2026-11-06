#include <bits/stdc++.h>

constexpr int64_t inf = 1e18;

struct Flow {
    struct Edge {
        int to, id;
        int64_t fl, cp;
    };

    int n, S, T;
    int64_t flow;
    std::vector<std::vector<Edge>> gr;
    std::vector<int> dist, used;

    Flow(int n, int S, int T) : n(n), S(S), T(T), flow(0) {
        gr.assign(n, {});
    }

    void add_edge(int u, int v, int64_t cp) {
        int a = gr[u].size(), b = gr[v].size();
        gr[u].push_back({v, b, 0, cp});
        gr[v].push_back({u, a, cp, cp});
    }

    void bfs(int s) {
        std::deque<int> deque;
        dist.assign(n, n);
        deque.push_back(s);
        dist[s] = 0;
        while (!deque.empty()) {
            int v = deque.front();
            deque.pop_front();
            for (auto e : gr[v]) {
                if (e.fl != e.cp) {
                    if (dist[e.to] == n) {
                        dist[e.to] = dist[v] + 1;
                        deque.push_back(e.to);
                    }
                }
            }
        }
    }

    int64_t dfs(int v, int64_t min) {
        if (v == T || min == 0) {
            return min;
        }
        for (int &i = used[v]; i < gr[v].size(); i++) {
            auto &e = gr[v][i];
            if (e.fl != e.cp && dist[e.to] == dist[v] + 1) {
                if (int64_t dt = dfs(e.to, std::min(min, e.cp - e.fl)); dt) {
                    e.fl += dt;
                    gr[e.to][e.id].fl -= dt;
                    return dt;
                }
            }
        }
        return 0;
    }

    int64_t find_max_flow() {
        while (bfs(S), dist[T] != n) {
            used.assign(n, 0);
            while (int64_t dlt = dfs(S, inf)) {
                flow += dlt;
            }
        }
        return flow;
    }
};
