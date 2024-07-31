#include <immintrin.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define TIME_DIFF_NS   ((___t2.tv_nsec + ___t2.tv_sec * (int64_t)1e9) - \
                        (___t1.tv_nsec + ___t1.tv_sec * (int64_t)1e9))
#define TIME_DIFF_MS   (TIME_DIFF_NS / (int64_t)1e6)
#define TIME_DIFF_MCS  (TIME_DIFF_NS / (int64_t)1e3)

#define unlikely(x) __builtin_expect(!!(x), 0)

static void Scalar(const int32_t *restrict l, const size_t lz,
                   const int32_t *restrict r, const size_t rz,
                   int32_t *restrict res) {
    const int32_t *const L = l + lz, *const R = r + rz;

    while ((L - l >= 2) & (R - r >= 2)) {

        register uint64_t lv = *(uint64_t *)l,
                          rv = *(uint64_t *)r,
                           v = (uint32_t)lv;

        bool le = (int32_t)lv <= (int32_t)rv;
        if (!le) v = (uint32_t)rv;  // `cmov`, please.
        l += le;
        r += le ^ 1;
        lv >>= (le ? 32 : 0);
        rv >>= (le ? 0 : 32);

        le = (int32_t)lv <= (int32_t)rv;
        v |= (uint64_t)(le ? (uint32_t)lv : (uint32_t)rv) << 32;
        l += le;
        r += le ^ 1;

        *(uint64_t *)res = v, res += 2;
    }

    while ((l < L) & (r < R)) {
        const bool le = *l <= *r;
        *res++ = le ? *l : *r, l += le, r += le ^ 1;
    }

    memcpy(res, l, (const void *)L - (const void *)l);
    memcpy(res, r, (const void *)R - (const void *)r);
}

static void SIMD(const int32_t *restrict lp, const size_t lz,
                 const int32_t *restrict rp, const size_t rz,
                 int32_t *restrict _res) {
    const int32_t *const L = lp + lz, *const R = rp + rz;

    const __m256i shl = _mm256_set_epi32(6, 5, 4, 3, 2, 1, 0, 7),
              reverse = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7),
                  one = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, 1),
                   lo = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, -1);

    while (((L - lp) >= 8) & ((R - rp) >= 8)) {

        const __m256i l = _mm256_loadu_si256((void *)lp),
                      r = _mm256_loadu_si256((void *)rp);
        __m256i li = _mm256_setzero_si256(),
                ri = _mm256_setzero_si256(),
               res = _mm256_setzero_si256();

        for (int step = 0; step < 8; step++) {
            const __m256i a = _mm256_permutevar8x32_epi32(l, li),
                          b = _mm256_permutevar8x32_epi32(r, ri);
            res = _mm256_or_si256(
                _mm256_permutevar8x32_epi32(res, shl),
                _mm256_and_si256(lo, _mm256_min_epi32(a, b))
            );
            const __m256i m = _mm256_and_si256(one, _mm256_cmpgt_epi32(a, b));
            li = _mm256_add_epi32(li, _mm256_xor_si256(one, m)),
            ri = _mm256_add_epi32(ri, m);
        }

        const int32_t lc = _mm256_cvtsi256_si32(li);
        _mm256_stream_si256(
            (void *)_res,
            _mm256_permutevar8x32_epi32(res, reverse)
        ), lp += lc, rp += 8 - lc, _res += 8;
    }

    Scalar(lp, L - lp, rp, R - rp, _res);
}

#define _(CND) \
    if (unlikely(CND)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        exit(1); \
    }

static int qcmp(const void *_a, const void *_b) {
    const int32_t *a = _a, *b = _b;
    return (*a > *b) - (*a < *b);
}

#define N (size_t)1e7

static int32_t L[N], R[N], A[N * 2];
static _Alignas(64) int32_t B[N * 2];

int main(void) {

    // srand(time(NULL));

    for (size_t i = 0; i < N; i++) L[i] = rand(), R[i] = rand();
    qsort(L, N, sizeof(int32_t), qcmp);
    qsort(R, N, sizeof(int32_t), qcmp);

    {
        volatile int32_t ramp = 1e9; while (--ramp > 0);
    }

    struct timespec ___t1, ___t2;
    clock_gettime(CLOCK_REALTIME, &___t1);
    Scalar(L, N, R, N, A);
    clock_gettime(CLOCK_REALTIME, &___t2);
    const size_t ms = TIME_DIFF_MS;

    clock_gettime(CLOCK_REALTIME, &___t1);
    SIMD(L, N, R, N, B);
    clock_gettime(CLOCK_REALTIME, &___t2);

    _(memcmp(A, B, sizeof(A)))

    printf(
        "\n"
        "\tScalar: %8zu ms\n"
        "\t  SIMD: %8zu ms\n"
        "\n"
    , ms, (size_t)TIME_DIFF_MS);

    return 0;
}
