#pragma GCC target("bmi2")
#include <immintrin.h>
#include <cstdint>
uint64_t interleave(uint64_t x) {
    // x = (x | (x << 16))& 0x0000'FFFF'0000'FFFF; x = (x | (x << 8)) & 0x00FF'00FF'00FF'00FF;
    // x = (x | (x << 4)) & 0x0F0F'0F0F'0F0F'0F0F; x = (x | (x << 2)) & 0x3333'3333'3333'3333;
    // x = (x | (x << 1)) & 0x5555'5555'5555'5555;
    const uint64_t mask = 0x5555'5555'5555'5555;
    x = _pdep_u64(x, mask) | _pdep_u64(x >> 32, mask) << 1;
    return x;
} // http://threadlocalmutex.com/?p=126 // for 64-bit indicies
uint64_t hilbertXYToIndex_logarithmic(uint32_t x, uint32_t y) {
    uint32_t A, B, C, D; // Initial prefix scan round, prime with x and y
    {   uint32_t a = x ^ y, b = 0xFFFF'FFFF ^ a, c = 0xFFFF'FFFF ^ (x | y), d = x & (y ^ 0xFFFF'FFFF);
        A = a | (b >> 1), B = (a >> 1) ^ a, C = ((c >> 1) ^ (b & (d >> 1))) ^ c;
        D = ((a & (c >> 1)) ^ (d >> 1)) ^ d;
    } { uint32_t a = A, b = B, c = C, d = D;
        A = ((a & (a >> 2)) ^ (b & (b >> 2))), B = ((a & (b >> 2)) ^ (b & ((a ^ b) >> 2)));
        C ^= ((a & (c >> 2)) ^ (b & (d >> 2))), D ^= ((b & (c >> 2)) ^ ((a ^ b) & (d >> 2)));
    } { uint32_t a = A, b = B, c = C, d = D;
        A = ((a & (a >> 4)) ^ (b & (b >> 4))), B = ((a & (b >> 4)) ^ (b & ((a ^ b) >> 4)));
        C ^= ((a & (c >> 4)) ^ (b & (d >> 4))), D ^= ((b & (c >> 4)) ^ ((a ^ b) & (d >> 4)));
    } { uint32_t a = A, b = B, c = C, d = D;
        A = ((a & (a >> 8)) ^ (b & (b >> 8))), B = ((a & (b >> 8)) ^ (b & ((a ^ b) >> 8)));
        C ^= ((a & (c >> 8)) ^ (b & (d >> 8))), D ^= ((b & (c >> 8)) ^ ((a ^ b) & (d >> 8)));
    } /* Final round and projection */{
        uint32_t a = A, b = B, c = C, d = D;
        C ^= ((a & (c >> 16)) ^ (b & (d >> 16))), D ^= ((b & (c >> 16)) ^ ((a ^ b) & (d >> 16)));
    } uint32_t a = C ^ (C >> 1), b = D ^ (D >> 1); // Undo transformation prefix scan
    uint32_t i0 = x ^ y, i1 = b | (0xFFFF'FFFF ^ (i0 | a)); // Recover index bits
    return (interleave(i1) << 1) | interleave(i0);
}
