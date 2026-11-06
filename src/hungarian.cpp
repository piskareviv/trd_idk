/* works in n^2 * m
set values to a[1..n][1..m] (n <= m)
run calc(n, m) to find MINIMUM weighted matching
permutation in ans[1..n]
-v[0] is answer
if int is not enough, vector<ll> v, u, return type -> ll
-v[0] and u[0] overflow, ellse not greate than max edge weight
else can be int
works with negative values
u, v are potentials, s.t. u[i] + v[j] <= a[i][j] and u[i] + v[j] equals to matching,
*/
const int N = 210, inf = 1e9 + 100;
int a[N][N];
int ans[N];
int calc(int n, int m) {
    ++n, ++m;
    vector<int> u(n), v(m), p(m), prev(m);
    for (int i = 1; i < n; ++i) {
        p[0] = i;
        int x = 0;
        vector<int> mn(m, inf);
        vector<int> was(m, 0);
        while (p[x]) {
            was[x] = 1;
            int ii = p[x], dd = inf, y = 0;
            for (int j = 1; j < m; ++j) if (!was[j]) {
                int cur = a[ii][j] - u[ii] - v[j];
                if (cur < mn[j]) mn[j] = cur, prev[j] = x;
                if (mn[j] < dd) dd = mn[j], y = j;
            }
            for (int j = 0; j < m; j++) {
                if (was[j]) u[p[j]] += dd, v[j] -= dd;
                else mn[j] -= dd;
            }
            x = y;
        }
        while (x) {
            int y = prev[x];
            p[x] = p[y];
            x = y;
        }
    }
    for (int j = 1; j < m; ++j)
        ans[p[j]] = j;
    return -v[0];
}
