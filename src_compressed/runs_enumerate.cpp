/* their number <= n, sum (r-l+1)/(2period) <= n, sum of (r-l+1) <= O(n log(n)) */
struct Run { int period; /* period is minimal period of [l, r], r - l >= 2period */ int l, r; /*
    for [l-1, r] and [l, r+1] cond is not satisfied */ Run() = default;
    Run(int period, int l, int r) : period(period), l(l), r(r) {}
    friend bool operator<(const Run& r1, const Run& r2) { if (r1.period != r2.period) { return
        r1.period < r2.period; } return r1.l != r2.l ? r1.l < r2.l : r1.r < r2.r; }
    friend bool operator==(const Run& r1, const Run& r2) { return r1.period == r2.period && r1.l
        == r2.l && r1.r == r2.r; }, };
/* nlogn, was 900ms for n=10^6 */
vector<Run> run_enumerate(string& s) { char sentinel = '#' ; for (auto& e : s) assert(e !=
    sentinel); vector<Run> glob_result;
    auto div_conq = [&](auto div_conq, int l, int r) -> vector<Run> { if (r - l <= 1) return {};
        const int m = (l + r) >> 1; vector<Run> run_l = div_conq(div_conq, l, m); vector<Run>
        run_r = div_conq(div_conq, m, r); string rl = s.substr(m, r - m); rl += sentinel; rl +=
        s.substr(l, m - l); vector<int> z_rl = z_function(rl); reverse(rl.begin(), rl.end());
        vector<int> z_rl_rev = z_function(rl); const int siz = rl.size(); vector<Run> result;
        auto add_ans = [&](Run &&run) { if (run.l == l || run.r == r) {
            result.emplace_back(move(run)); } else { glob_result.emplace_back(move(run)); } };
        const int len_l = m - l, len_r = r - m; vector<Run> run_m(len_r / 2 + 1); for (auto
            &run: run_r) { if (run.l != m) { add_ans(move(run)); continue; } run_m[run.period] =
            move(run); }
        for (auto &run: run_l) { if (run.r != m) { add_ans(move(run)); continue; } const int
            period = run.period; if (z_rl[siz - period] == period) { if (run_m[period].period) {
            run.r = run_m[period].r; run_m[period] = Run{}; add_ans(move(run)); } else { run.r =
            m + period + z_rl[period]; add_ans(move(run)); } } else { run.r = m + z_rl[siz -
            period]; add_ans(move(run)); } }
        for (auto &run: run_m) { if (run.period) { const int period = run.period; if (z_rl[siz -
            period] == period) { if (2 * period <= len_l && z_rl[siz - 2 * period] >= period)
            continue; run.l = m - period - z_rl_rev[period]; add_ans(move(run)); } else { run.l
            = m - z_rl_rev[siz - period]; add_ans(move(run)); } } }
        for (int period = 1; period <= len_l; ++period) { bool skip_r = 2 * period <= len_r &&
            z_rl[period] >= period; bool skip_l = 2 * period <= len_l && z_rl[siz - 2 * period]
            >= period; if (z_rl[siz - period] == period) { if (skip_l || skip_r) continue; const
            int beg_pos = m - period - z_rl_rev[period]; const int end_pos = m + period +
            z_rl[period]; add_ans(Run{ period, beg_pos, end_pos }); } else { if (!skip_r) {
            const int beg_pos = m - z_rl_rev[siz - period]; const int end_pos = m + period +
            z_rl[period]; if (end_pos - beg_pos >= 2 * period) { add_ans(Run{ period, beg_pos,
            end_pos }); } } if (!skip_l) { const int beg_pos = m - period - z_rl_rev[period];
            const int end_pos = m + z_rl[siz - period]; if (end_pos - beg_pos >= 2 * period) {
            add_ans(Run{ period, beg_pos, end_pos }); } } } }
        return result; }; const int n = s.size(); vector<tuple<int, int, int>> runs; for (Run&
            run : div_conq(div_conq, 0, n)) { runs.emplace_back(run.l, run.r, run.period); }
    for (Run& run : glob_result) { runs.emplace_back(run.l, run.r, run.period); }
    sort(begin(runs), end(runs));
    runs.erase(
    unique( begin(runs), end(runs),
    [](auto& r1, auto& r2) { return get<0>(r1) == get<0>(r2) && get<1>(r1) == get<1>(r2); }
    ), end(runs) ); vector<Run> res; for (auto& [l, r, t] : runs) res.emplace_back(t, l, r);
        return res; }
