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

static void Scalar(int32_t *l, const size_t n) {

    for (int32_t t, *r = (l + n) - 1; l < r; l++, r--)
        t = *l, *l = *r, *r = t;
}

static void SIMD_1(int32_t *l, const size_t n) {

    const __m256i rev = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);

    for (int32_t *r = (l + n) - 8; l + 8 <= r; l += 8, r -= 8) {
        __m256i v1 = _mm256_load_si256((void *)l),
                v2 = _mm256_load_si256((void *)r);
        v1 = _mm256_permutevar8x32_epi32(v1, rev),
        v2 = _mm256_permutevar8x32_epi32(v2, rev);
        _mm256_store_si256((void *)l, v2),
        _mm256_store_si256((void *)r, v1);
    }
}

static void SIMD_3(int32_t *l, const size_t n) {

    const __m256i rev = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);

    for (int32_t *r = (l + n) - 24; l + 24 <= r; l += 24, r -= 24) {
        __m256i v1 = _mm256_load_si256((void *)l),
                v2 = _mm256_load_si256((void *)(l + 8)),
                v3 = _mm256_load_si256((void *)(l + 16)),
                v4 = _mm256_load_si256((void *)r),
                v5 = _mm256_load_si256((void *)(r + 8)),
                v6 = _mm256_load_si256((void *)(r + 16));
        v1 = _mm256_permutevar8x32_epi32(v1, rev),
        v2 = _mm256_permutevar8x32_epi32(v2, rev),
        v3 = _mm256_permutevar8x32_epi32(v3, rev),
        v4 = _mm256_permutevar8x32_epi32(v4, rev),
        v5 = _mm256_permutevar8x32_epi32(v5, rev),
        v6 = _mm256_permutevar8x32_epi32(v6, rev);
        _mm256_store_si256((void *)l, v6),
        _mm256_store_si256((void *)(l + 8), v5),
        _mm256_store_si256((void *)(l + 16), v4),
        _mm256_store_si256((void *)r, v3),
        _mm256_store_si256((void *)(r + 8), v2),
        _mm256_store_si256((void *)(r + 16), v1);
    }
}

#define _(CND) \
    if (unlikely(CND)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        exit(1); \
    }

#define N (size_t)96e6
_Static_assert(N % 48 == 0, "");

static _Alignas(64) int32_t A[N], B[N], C[N];

int main(void) {

    // srand(time(NULL));

    for (size_t i = 0; i < N; i++) A[i] = B[i] = C[i] = rand();

    {
        volatile int32_t ramp = 1e9; while (--ramp > 0);
    }

    struct timespec ___t1, ___t2;
    clock_gettime(CLOCK_REALTIME, &___t1);
    Scalar(A, N);
    clock_gettime(CLOCK_REALTIME, &___t2);
    const size_t ms = TIME_DIFF_MS;

    clock_gettime(CLOCK_REALTIME, &___t1);
    SIMD_1(B, N);
    clock_gettime(CLOCK_REALTIME, &___t2);
    const size_t ms_1 = TIME_DIFF_MS;
    _(memcmp(A, B, sizeof(A)))

    clock_gettime(CLOCK_REALTIME, &___t1);
    SIMD_3(C, N);
    clock_gettime(CLOCK_REALTIME, &___t2);
    _(memcmp(A, C, sizeof(A)))

    printf(
        "\n"
        "\tScalar: %8zu ms\n"
        "\tSIMD_1: %8zu ms\n"
        "\tSIMD_3: %8zu ms\n"
        "\n"
    , ms, ms_1, (size_t)TIME_DIFF_MS);

    return 0;
}
