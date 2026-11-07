typedef bitset<128> B;
template <class F>
void cliques(vector<B>& eds, F f, B P = ~B(), B X = {}, B R = {}) {
    if (!P.any()) {
        if (!X.any()) f(R);
        return;
    }
    auto q = (P | X)._Find_first();
    auto cands = P & ~eds[q];
    rep(i, 0, sz(eds)) if (cands[i]) {
        R[i] = 1;
        cliques(eds, f, P & eds[i], X & eds[i], R);
        R[i] = P[i] = 0;
        X[i] = 1;
    }
}