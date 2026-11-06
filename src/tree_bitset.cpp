#include <iostream>
#pragma GCC optimize("O3")
#pragma GCC target("avx2,lzcnt,bmi,bmi2")
#include <bits/stdc++.h>

template <typename u_tp = uint64_t>
class TreeBitset {
   private:
    static constexpr size_t B = sizeof(u_tp) * 8;

    std::vector<u_tp> my_data;
    std::vector<u_tp*> data;
    size_t n, lg;

   public:
    TreeBitset(size_t n = 0) {
        assign(n);
    }

    void assign(size_t n) {
        this->n = n;
        size_t m = n + 2;
        std::vector<size_t> vec;
        while (m > 1) {
            m = (m - 1) / B + 1;
            vec.push_back(m);
        }
        std::reverse(vec.begin(), vec.end());

        lg = vec.size();
        data.resize(vec.size());
        size_t sum = std::accumulate(vec.begin(), vec.end(), size_t(0));
        my_data.assign(sum, 0);
        for (size_t i = 0, s = 0; i < lg; s += vec[i], i++) {
            data[i] = my_data.data() + s;
        }

        for (size_t i = 0, k = lg; k--; i /= B) {
            data[k][i / B] |= u_tp(1) << i % B;
        }
        for (size_t i = n + 1, k = lg; k--; i /= B) {
            data[k][i / B] |= u_tp(1) << i % B;
        }
    }

    size_t size() const {
        return n;
    }

    void clear() {
        my_data.assign(my_data.size(), 0);
        for (size_t i = 0, k = lg; k--; i /= B) {
            data[k][i / B] |= u_tp(1) << i % B;
        }
        for (size_t i = n + 1, k = lg; k--; i /= B) {
            data[k][i / B] |= u_tp(1) << i % B;
        }
    }

    // i must be in [0, n)
    bool insert(size_t i) {
        i++;
        if ((data[lg - 1][i / B] >> i % B) & 1) {
            return false;
        }
        for (size_t k = lg; k--; i /= B) {
            data[k][i / B] |= u_tp(1) << i % B;
        }
        return true;
    }

    // i must be in [0, n)
    bool erase(size_t i) {
        i++;
        if (!((data[lg - 1][i / B] >> i % B) & 1)) {
            return false;
        }
        data[lg - 1][i / B] ^= u_tp(1) << i % B;
        i /= B;
        for (size_t k = lg - 1; k > 0 && !data[k][i]; k--, i /= B) {
            data[k - 1][i / B] ^= u_tp(1) << i % B;
        }
        return true;
    }

    // i must be in [0, n)
    bool contains(size_t i) const {
        i++;
        return (data[lg - 1][i / B] >> i % B) & 1;
    }

    // i must be in [0, n]
    // smallest element greater than or equal to i, n if doesn't exist
    size_t find_next(size_t i) const {
        i++;
        size_t k = lg - 1;

        for (; !u_tp(data[k][i / B] >> i % B); k--) {
            i = i / B + 1;
        }

        for (; k < lg; k++) {
            u_tp mask = u_tp(data[k][i / B] >> i % B) << i % B;
            size_t ind = std::countr_zero(mask);
            i = (i / B * B + ind) * B;
        }
        i /= B;
        return i - 1;
    }

    // i must be in [0, n)
    // largest element less than or equal to i, n if doesn't exist
    size_t find_prev(size_t i) const {
        i++;
        size_t k = lg - 1;
        for (; !u_tp(data[k][i / B] << (B - i % B - 1)); k--) {
            i = i / B - 1;
        }

        for (; k < lg; k++) {
            u_tp mask = u_tp(data[k][i / B] << (B - i % B - 1)) >> (B - i % B - 1);
            assert(mask);
            size_t ind = B - 1 - std::countl_zero(mask);
            i = (i / B * B + ind) * B + (B - 1);
        }
        i /= B;
        if (i == 0) {
            return n;
        }
        return i - 1;
    }
};
