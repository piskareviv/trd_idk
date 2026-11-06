#include <bits/stdc++.h>

using u32 = uint32_t;

constexpr u32 mod = 998244353;
constexpr u32 pr_root = 3;

u32 add(u32 a, u32 b) {
    return a + b - mod * (a + b >= mod);
}
void add_to(u32 &a, u32 b) {
    a = add(a, b);
}
u32 mul(u32 a, u32 b) {
    return a * 1ULL * b % mod;
}

u32 power(u32 b, u32 e) {
    u32 r = 1;
    for (; e > 0; e >>= 1) {
        if (e & 1) {
            r = mul(r, b);
        }
        b = mul(b, b);
    }
    return r;
}

int bit_ceil(int n) {
    int k = 1;
    while (k < n)
        k *= 2;
    return k;
}

int64_t total = 0;

struct NTT {
    std::vector<u32> wd, wrd;

    NTT() {
        int lg = __builtin_ctz(mod - 1);

        wd.assign(lg + 1, 0), wrd.assign(lg + 1, 0);
        for (int i = 0; i + 2 <= lg; i++) {
            u32 a = power(pr_root, (mod - 1 >> i + 2));
            u32 b = power(pr_root, (mod - 1 >> i + 2) * ((2 << i) - 2));

            u32 f = mul(a, power(b, mod - 2));

            wd[i] = f;
            wrd[i] = power(f, mod - 2);
        }
    }

    template <bool transposed>
    void butterfly_x2(u32 &a, u32 &b, u32 w) const {
        if (!transposed) {
            u32 a2 = a, b2 = mul(b, w);
            a = add(a2, b2), b = add(a2, mod - b2);
        } else {
            u32 a2 = add(a, b), b2 = mul(add(a, mod - b), w);
            a = a2, b = b2;
        }
    }

    template <bool inverse>
    void transform(int lg, u32 *data) const {
        total += 1 << lg;
        // std::cerr << "transform  " << inverse << "  " << lg << "\n";
        for (int k = inverse ? 0 : lg - 1; inverse ? k < lg : k >= 0; inverse ? k++ : k--) {
            u32 wi = 1;
            for (int i = 0; i < (1 << lg); i += (1 << k + 1)) {
                for (int j = 0; j < (1 << k); j++) {
                    butterfly_x2<inverse>(data[i + j], data[i + (1 << k) + j], wi);
                }
                wi = mul(wi, (inverse ? wrd : wd)[__builtin_ctz(~i >> k + 1)]);
            }
        }
        if (inverse) {
            u32 f = power(mod + 1 >> 1, lg);
            for (int i = 0; i < (1 << lg); i++) {
                data[i] = mul(data[i], f);
            }
        }
    }

    void dot(int n, const u32 *a, const u32 *b, u32 *c) const {
        for (int i = 0; i < n; i++) {
            c[i] = mul(a[i], b[i]);
        }
    }

    std::vector<u32> poly_mul(int n, std::vector<u32> a, std::vector<u32> b) {
        int lg = __builtin_ctz(bit_ceil(std::max<int>(1, (int)a.size() + (int)b.size() - 1)));
        a.resize(1 << lg), b.resize(1 << lg);
        transform<false>(lg, a.data());
        transform<false>(lg, b.data());
        dot(1 << lg, a.data(), b.data(), a.data());
        transform<true>(lg, a.data());

        a.resize(n);
        return a;
    }

    std::vector<u32> inv(int n, std::vector<u32> vec) {
        assert(vec.size() && vec[0] != 0);
        std::vector<u32> res = {power(vec[0], mod - 2)};

        std::vector<u32> tmp;
        for (int k = 0; (1 << k) < n; k++) {
            int m = 1 << k;

            res.resize(4 * m);
            tmp.assign(4 * m, 0);
            std::copy(vec.begin(), vec.begin() + std::min<int>(vec.size(), 2 * m), tmp.begin());

            transform<false>(k + 2, res.data());
            transform<false>(k + 2, tmp.data());
            for (int i = 0; i < (1 << k + 2); i++) {
                res[i] = mul(res[i], add(2, mod - mul(res[i], tmp[i])));
            }
            transform<true>(k + 2, res.data());
            res.resize(1 << k + 1);
        }

        res.resize(n);
        return res;
    }

    std::vector<u32> deriv(std::vector<u32> vec) {
        assert(vec.size() > 0);
        for (int i = 1; i < vec.size(); i++) {
            vec[i - 1] = mul(vec[i], i);
        }
        vec.pop_back();
        return vec;
    }

    std::vector<u32> integ(std::vector<u32> vec) {
        static std::vector<u32> inv = {0};
        while (inv.size() <= vec.size()) {
            inv.push_back(power(inv.size(), mod - 2));
        }

        vec.push_back(0);
        for (int i = vec.size() - 1; i >= 1; i--) {
            vec[i] = mul(vec[i - 1], inv[i]);
        }
        vec[0] = 0;
        return vec;
    }

    std::vector<u32> ln(int n, std::vector<u32> vec) {
        assert(vec[0] == 1);
        return integ(poly_mul(n - 1, deriv(vec), inv(n - 1, vec)));
    }

    std::vector<u32> sub(const std::vector<u32> &a, const std::vector<u32> &b) {
        std::vector<u32> res(std::max(a.size(), b.size()));
        for (int i = 0; i < res.size(); i++) {
            res[i] = add(((i < a.size()) ? a[i] : 0), mod - ((i < b.size()) ? b[i] : 0));
        }
        return res;
    }

    std::vector<u32> exp(int n, std::vector<u32> vec) {
        std::vector<u32> res = {1};
        for (int k = 0; (1 << k) < n; k++) {
            int m = 1 << k;
            std::vector<u32> tmp(vec.begin(), vec.begin() + std::min<int>(vec.size(), 1 << k + 1));
            // res = sub(res, poly_mul(2 * m, res, sub(ln(2 * m, res), tmp)));
            // res = sub(res, poly_mul(2 * m, res, sub(ln(2 * m, res), tmp)));
            res = poly_mul(2 * m, res, sub({1}, sub(ln(2 * m, res), tmp)));
        }
        return res;
    }
} ntt;

std::vector<u32> inverse = {0};
std::vector<u32> factorial = {1}, inv_factorial = {1};

void expand(int n) {
    while (inverse.size() <= n) {
        int k = inverse.size();
        inverse.push_back(power(k, mod - 2));
        factorial.push_back(mul(factorial.back(), k));
        inv_factorial.push_back(mul(inv_factorial.back(), inverse[k]));
    }
}

int C(int n, int k) {
    return mul(factorial[n], mul(inv_factorial[k], inv_factorial[n - k]));
}
