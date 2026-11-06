#include <bits/stdc++.h>
#include <immintrin.h>

__m256i compress_epi32(__m256i val, __m256i mask) {
    uint32_t i32 = _mm256_movemask_epi8(mask);
    constexpr uint32_t identity = 0x76543210;
    i32 = _pext_u32(identity, i32);
    constexpr uint64_t expand = 0x0F0F0F0F0F0F0F0Full;
    uint64_t i64 = _pdep_u64(i32, expand);
    __m128i vec = _mm_cvtsi64_si128((int64_t)i64);
    __m256i perm = _mm256_cvtepu8_epi32(vec);

    return _mm256_permutevar8x32_epi32(val, perm);
}

inline void merge_epi32(__m256i& a, __m256i& b) {
    {
        static const __m256i xr = _mm256_set_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
        const __m256i rev_perm = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);
        b = _mm256_permutevar8x32_epi32(b, rev_perm);
        __m256i vcmp = _mm256_cmpgt_epi32(a, b);
        vcmp = _mm256_xor_si256(vcmp, xr);
        __m256i tmp_a = _mm256_blendv_epi8(a, b, vcmp);
        __m256i tmp_b = _mm256_blendv_epi8(b, a, vcmp);
        a = tmp_a;
        b = tmp_b;
        b = _mm256_permute2x128_si256(b, a, 0b0000'0001);
    }
    {
        static const __m256i xr = _mm256_set_epi32(-1, -1, 0, 0, -1, -1, 0, 0);
        __m256i vcmp = _mm256_cmpgt_epi32(a, b);
        vcmp = _mm256_xor_si256(vcmp, xr);
        __m256i tmp_a = _mm256_blendv_epi8(a, b, vcmp);
        __m256i tmp_b = _mm256_blendv_epi8(b, a, vcmp);
        a = tmp_a;
        b = tmp_b;
        b = _mm256_shuffle_epi32(b, _MM_SHUFFLE(1, 0, 3, 2));
    }
    {
        static const __m256i xr = _mm256_set_epi32(-1, 0, -1, 0, -1, 0, -1, 0);
        __m256i vcmp = _mm256_cmpgt_epi32(a, b);
        vcmp = _mm256_xor_si256(vcmp, xr);
        __m256i tmp_a = _mm256_blendv_epi8(a, b, vcmp);
        __m256i tmp_b = _mm256_blendv_epi8(b, a, vcmp);
        a = tmp_a;
        b = tmp_b;
        b = _mm256_shuffle_epi32(b, _MM_SHUFFLE(2, 3, 0, 1));
    }
    {
        // static const __m256i xr = _mm256_set_epi32(-1, 0, -1, 0, -1, 0, -1, 0);
        __m256i vcmp = _mm256_cmpgt_epi32(a, b);
        // vcmp = _mm256_xor_si256(vcmp, xr);
        __m256i tmp_a = _mm256_blendv_epi8(a, b, vcmp);
        __m256i tmp_b = _mm256_blendv_epi8(b, a, vcmp);
        a = tmp_a;
        b = tmp_b;
    }
    {
        __m256i tmp_a = _mm256_unpacklo_epi32(a, b);
        __m256i tmp_b = _mm256_unpackhi_epi32(a, b);
        // a = tmp_a;
        // b = tmp_b;
        a = _mm256_permute2x128_si256(tmp_a, tmp_b, 0b0010'0000);
        b = _mm256_permute2x128_si256(tmp_a, tmp_b, 0b0011'0001);
    }
}