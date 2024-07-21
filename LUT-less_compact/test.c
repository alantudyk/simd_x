#include <immintrin.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define unlikely(x) __builtin_expect(!!(x), 0)

static size_t select_lt(const int32_t *s,
                        const size_t sz,
                        const int32_t _t,
                        int32_t *const _c) {

    if (unlikely(_t == INT32_MIN)) return 0;

    int32_t *c = _c;
    const __m256i t = _mm256_set1_epi32(_t - 1),
                shr = _mm256_slli_epi32(
        _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), 2
    );

    for (const int32_t *const S = s + sz; s < S; s += 8) {
        const __m256i a = _mm256_load_si256((void *)s);
        const uint32_t m = ~_mm256_movemask_epi8(_mm256_cmpgt_epi32(a, t));
        const __m256i p = _mm256_set1_epi32(_pext_u32(0x76543210, m));
        _mm256_storeu_si256(
            (void *)c,
            _mm256_permutevar8x32_epi32(a, _mm256_srlv_epi32(p, shr))
        );
        c += __builtin_popcount(m) >> 2;
    }

    return c - _c;
}

#define _(CND) \
    if (unlikely(CND)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        exit(1); \
    }

#define N (size_t)1e6
_Static_assert(N % 16 == 0);
#define T 555
static _Alignas(64) int32_t A[N], B[N], C[N];

int main(void) {

    // srand(time(NULL));

    size_t bz = 0;
    for (size_t i = 0; i < N; i++)
        if ((A[i] = rand() % (T * 2)) < T)
            B[bz++] = A[i];

    const size_t cz = select_lt(A, N, T, C);
    _(bz != cz || !!memcmp(B, C, bz * sizeof(int32_t)))

    printf(
        "\n"
        "\t%zu values are less than %d\n"
        "\n",
    bz, T);

    return 0;
}
