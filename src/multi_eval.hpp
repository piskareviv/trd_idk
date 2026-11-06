// https://judge.yosupo.jp/submission/243168

#include <bits/stdc++.h>
#include <immintrin.h>

size_t ntt_sum_size = 0;

using u32 = uint32_t;
using u64 = uint64_t;

struct WTF {
    static constexpr u32 mod = 998'244'353;
    static constexpr u32 pr_root = 3;
    static constexpr int LG = 32;

    u32 wd[LG], wrd[LG];
    u32 w_rt[LG], wr_rt[LG];

    u32 add(u32 a, u32 b) { return a + b - mod * (a + b >= mod); }
    void add_to(u32& a, u32 b) { a = add(a, b); }
    u32 mul(u32 a, u32 b) { return u64(a) * b % mod; }
    u32 power(u32 b, u32 e) {
        u32 r = 1;
        for (; e > 0; e >>= 1) {
            if (e & 1) r = mul(r, b);
            b = mul(b, b);
        }
        return r;
    }

    WTF() {
        int lg = __builtin_ctz(mod - 1) + 1;
        for (int i = 0; i < std::min(lg, LG); i++) {
            u32 wi = power(pr_root, mod - 1 >> i + 2);
            u32 rm = power(pr_root, (mod - 1 >> i + 1) * ((1 << i) - 1));
            u32 w_dlt = mul(wi, power(rm, mod - 2));
            w_rt[i] = wi, wr_rt[i] = power(wi, mod - 2);
            wd[i] = w_dlt, wrd[i] = power(w_dlt, mod - 2);
        }
    }

    template <bool transposed>
    void butterfly_x2(u32* ptr_a, u32* ptr_b, u32 w) {
        u32 a = *ptr_a, b = *ptr_b, a2, b2;
        if (!transposed) {
            u32 c = mul(b, w);
            a2 = add(a, c), b2 = add(a, mod - c);
        } else {
            a2 = add(a, b), b2 = mul(add(a, mod - b), w);
        }
        *ptr_a = a2, *ptr_b = b2;
    }

    template <bool inverse = false, bool right_part = false>
    void transform(int lg, u32* data) {
        ntt_sum_size += 1 << lg;
        for (int k = !inverse ? lg - 1 : 0; !inverse ? k >= 0 : k < lg; !inverse ? k-- : k++) {
            u32 wi = right_part ? (inverse ? wr_rt : w_rt)[lg - 1 - k] : 1;
            for (int i = 0; i < (1 << lg); i += (1 << k + 1)) {
                for (int j = 0; j < (1 << k); j++) {
                    butterfly_x2<inverse>(data + i + j, data + i + (1 << k) + j, wi);
                }
                wi = mul(wi, (!inverse ? wd : wrd)[__builtin_ctz(~i >> k + 1)]);
            }
        }
        if (inverse) {
            u32 f = power(mod + 1 >> 1, lg);
            for (int i = 0; i < (1 << lg); i++) {
                data[i] = mul(data[i], f);
            }
        }
    }

    std::vector<u32> inv_fps(std::vector<u32> vec) {
        assert(vec.size() && vec[0] != 0);
        int k = 0;
        std::vector<u32> inv = {power(vec[0], mod - 2)};
        std::vector<u32> tmp1, tmp2, tmp3;
        while ((1 << k) < vec.size()) {
            int n = 1 << k;
            vec.resize(std::max<int>(vec.size(), 2 * n));

            tmp1.assign(2 * n, 0);
            std::copy(inv.begin(), inv.begin() + n, tmp1.begin());
            transform<false>(k + 1, tmp1.data());

            tmp2.assign(2 * n, 0);
            std::copy(vec.begin(), vec.begin() + 2 * n, tmp2.begin());
            transform<false>(k + 1, tmp2.data());

            // const u32 fix = power(mod + 1 >> 1, k + 1);
            for (int i = 0; i < 2 * n; i++) {
                tmp2[i] = mul(tmp1[i], tmp2[i]);
            }
            transform<true>(k + 1, tmp2.data());
            for (int i = 0; i < n; i++) {
                tmp2[i] = 0;
            }
            transform<false>(k + 1, tmp2.data());
            for (int i = 0; i < 2 * n; i++) {
                tmp1[i] = mul(tmp1[i], add(1, mod - tmp2[i]));
            }
            transform<true>(k + 1, tmp1.data());
            inv.resize(2 * n);
            std::copy(tmp1.begin() + n, tmp1.begin() + 2 * n, inv.begin() + n);
            k++;
        }

        inv.resize(vec.size());
        return inv;
    }

    std::vector<u32> evaluate(std::vector<u32> poly, std::vector<u32> points) {
        int res_sz = points.size();
        int n = std::max(poly.size(), points.size());
        int lg = std::__lg(std::max<int>(n - 1, 1)) + 1;  // * doesn't work for lg = 0
        poly.resize(1 << lg), points.resize(1 << lg);
        std::vector<std::vector<u32>> data(lg + 1, std::vector<u32>(1 << lg + 1));
        for (int i = 0; i < (1 << lg); i++) {
            data[0][2 * i] = add(0, mod + 1 - points[i]);
            data[0][2 * i + 1] = add(0, mod - 1 - points[i]);
        }
        for (int k = 0; k < lg; k++) {
            for (int i = 0; i < (1 << lg + 1); i += 1 << k + 2) {
                for (int j = 0; j < (1 << k + 1); j++) {
                    data[k + 1][i + j] = mul(data[k][i + j], data[k][i + (1 << k + 1) + j]);
                }
                if (k + 1 != lg) {
                    std::copy(data[k + 1].begin() + i, data[k + 1].begin() + i + (1 << k + 1),
                              data[k + 1].begin() + i + (1 << k + 1));
                    transform<true>(k + 1, data[k + 1].data() + i + (1 << k + 1));
                    add_to(data[k + 1][i + (1 << k + 1)], mod - 2);
                    transform<false, true>(k + 1, data[k + 1].data() + i + (1 << k + 1));
                } else {
                    transform<true>(k + 1, data[k + 1].data() + i);
                    add_to(data[k + 1][i], mod - 1);
                    add_to(data[k + 1][i + (1 << k + 1)], 1);
                }
            }
        }

        std::vector<u32> dt = std::move(data[lg]);

        std::reverse(dt.begin(), dt.begin() + (1 << lg) + 1);
        dt.resize(1 << lg);

        dt = inv_fps(dt);
        std::reverse(dt.begin(), dt.end());

        dt.resize(1 << lg + 1);
        transform<false>(lg + 1, dt.data());

        poly.resize(1 << lg + 1);
        std::rotate(poly.begin(), poly.begin() + (1 << lg + 1) - 1, poly.end());
        transform<false>(lg + 1, poly.data());
        for (int i = 0; i < (1 << lg + 1); i++) {
            dt[i] = mul(dt[i], poly[i]);
        }

        for (int k = lg - 1; k >= 0; k--) {
            for (int i = 0; i < (1 << lg + 1); i += (1 << k + 2)) {
                transform<true, true>(k + 1, dt.data() + i + (1 << k + 1));
                transform<false>(k + 1, dt.data() + i + (1 << k + 1));
                for (int j = 0; j < (1 << k + 1); j++) {
                    u32 val = add(dt[i + j], mod - dt[i + (1 << k + 1) + j]);
                    dt[i + (1 << k + 1) + j] = mul(val, data[k][i + j]);
                    dt[i + j] = mul(val, data[k][i + (1 << k + 1) + j]);
                }
            }
        }

        std::vector<u32> ans(1 << lg);
        u32 fix = power(mod + 1 >> 1, lg + 1);
        for (int i = 0; i < (1 << lg); i++) {
            ans[i] = add(dt[2 * i], mod - dt[2 * i + 1]);
            ans[i] = mul(ans[i], fix);
        }
        ans.resize(res_sz);
        return ans;
    }
};
