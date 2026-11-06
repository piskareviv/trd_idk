ld integrate(ld L, ld R) { /* g(x) should be declared */ const int ITERS = 5e5; ld ans = 0; ld
    step = (R - L) / ITERS; for (int it = 0; it < ITERS; it++) { ld xl = L + step * it; ld xr =
    L + step * (it + 1); ld x1 = (xl + xr) / 2; ld x0 = x1 - (x1 - xl) * sqrt(3.0 / 5); ld x2 =
    x1 + (x1 - xl) * sqrt(3.0 / 5); ans += (5 * g(x0) + 8 * g(x1) + 5 * g(x2)) / 18 * step; }
    return ans; }
