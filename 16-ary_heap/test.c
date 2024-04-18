#include <stdio.h>
#include "heap.h"

#include <immintrin.h>

int main(void) {

    const __m256i P2 = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1),
                  P4 = _mm256_set_epi32(4, 4, 6, 6, 0, 0, 2, 2),
                  P8 = _mm256_set_epi32(0, 0, 0, 0, 4, 4, 4, 4);

    int x[16] = {
         7, 2, 9, 5, 4, 5, 2, 2,
         7, 8, 7, 3, 7, 9, 9, 0,
    };

    const __m256i a = _mm256_loadu_si256((void *)x),
                  b = _mm256_loadu_si256((void *)(x + 8));

    __m256i m = _mm256_min_epi32(a, b), p;

    p = _mm256_permutevar8x32_epi32(m, P2), m = _mm256_min_epi32(m, p);
    p = _mm256_permutevar8x32_epi32(m, P4), m = _mm256_min_epi32(m, p);
    p = _mm256_permutevar8x32_epi32(m, P8), m = _mm256_min_epi32(m, p);

    uint32_t lo = _mm256_movemask_epi8(_mm256_cmpeq_epi32(a, m));
    uint32_t hi = _mm256_movemask_epi8(_mm256_cmpeq_epi32(b, m));

    printf("%d\n", __builtin_ctzll(((uint64_t)hi << 32) | lo) / 4);

    return 0;
}
