#include <bits/stdc++.h>

using u64 = uint64_t;
using u128 = __uint128_t;

u64 add(u64 a, u64 b, u64 mod) {
    return a + b - mod * (a + b >= mod);
}
u64 mul(u64 a, u64 b, u64 mod) {
    return u128(a) * b % mod;
}
u64 power(u64 b, u64 e, u64 mod) {
    u64 r = 1;
    for (; e > 0; e >>= 1) {
        if (e & 1) {
            r = mul(r, b, mod);
        }
        b = mul(b, b, mod);
    }
    return r;
}

u64 is_prime(u64 val) {
    if (val % 2 == 0) {
        return val == 2;
    }
    int lg = __builtin_ctzll(val - 1);
    for (u64 a : std::array{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 39, 41}) {
        u64 pw = power(a, val - 1 >> lg, val);
        for (int i = 0; i < lg && pw != 1; i++) {
            u64 pw2 = mul(pw, pw, val);
            if (i + 1 == lg && pw2 != 1) {
                return false;
            }
            if (pw2 == 1 && pw != val - 1) {
                return false;
            }
            pw = pw2;
        }
    }
    return true;
}

u64 find_divisor(u64 val) {
    for (int i = 2; i <= 1000; i++) {
        if (val % i == 0) {
            if (val == i) {
                return 0;
            }
            return i;
        }
    }
    if (val <= 1e6 || is_prime(val)) {
        return 0;
    }
    auto f = [&](u64 x) {
        return add(mul(x, x, val), 3, val);
    };
    static std::mt19937_64 rnd;
    u64 a = rnd() % val;
    u64 b = a;
    std::vector<int64_t> vec;
    for (int64_t it = 0;; it++) {
        a = f(a);
        b = f(f(b));
        u64 diff = std::max(a, b) - std::min(a, b);
        vec.push_back(diff);
        if (vec.size() >= 200) {
            u64 prod = 1;
            for (u64 i : vec) {
                if (prod != 0) {
                    prod = mul(prod, i, val);
                }
            }
            if (std::gcd(prod, val) != 1) {
                for (u64 i : vec) {
                    if (std::gcd(i, val) != 1) {
                        return std::gcd(i, val);
                    }
                }
                assert(false);
            }
            vec.clear();
        }
    }
}
