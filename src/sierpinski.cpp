int64_t get_cum_rec(int x, int y, int lg = 30) {
    if (lg == 0) return 0;
    x &= (1 << lg) - 1, y &= (1 << lg) - 1;
    const int i = lg - 1,  int sz = 1 << i;
    const int64_t all = int64_t(1) << 2 * i;
    int a = x >> i & 1, b = y >> i & 1, x2 = x & sz - 1, y2 = y & sz - 1;
    int64_t res = 0;
    if (a == 0 && b == 0) {
        if (x > y || x == y && x != 0 && x % 2 == 0) res += all * 3;
        return res + get_cum_rec(x2, y2, lg - 1); 
    } 
    res += all / 2 + 1;
    if (a == 1 && b == 1) {
        res += all; if (x2 == 0 && y2 == 0) return res + (all - 1);
        return res + get_cum_rec(x2, y2, lg - 1) - 1;
    } else if (a == 0 && b == 1) {
        if (x2 == sz - 1 && y2 == 0) return res;
        return res + all - get_cum_rec(sz - 1 - x2, y2, lg - 1);
    } else { 
        res += all * 2; if (x2 == 0 && y2 == sz - 1) return res;
        return res + all - get_cum_rec(x2, sz - 1 - y2, lg - 1);
    }
}
