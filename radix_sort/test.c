#include <immintrin.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define TIME_DIFF_NS   ((___t2.tv_nsec + ___t2.tv_sec * (int64_t)1e9) - \
                        (___t1.tv_nsec + ___t1.tv_sec * (int64_t)1e9))
#define TIME_DIFF_MS   (TIME_DIFF_NS / (int64_t)1e6)
#define TIME_DIFF_MCS  (TIME_DIFF_NS / (int64_t)1e3)

#define _(CND) \
    if (__builtin_expect(!!(CND), 0)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        return 1; \
    }

static void Scalar(uint32_t *const A,
                   const size_t n,
                   uint32_t *const B) {
    if (n < 2) return;
    _Alignas(64) size_t F[4][256] = {};
    for (size_t i = 0; i < n; i++) {
        const uint32_t x = A[i];
        ++F[0][(uint8_t)x];
        ++F[1][(uint8_t)(x >> 8)];
        ++F[2][(uint8_t)(x >> 16)];
        ++F[3][x >> 24];
    }
    uint32_t *a = A, *b = B, *tmp;
    for (size_t i = 0; i < 4; i++) {
        size_t o = 0, t, *const f = F[i];
        for (size_t j = 0; j < 256; j++)
            t = f[j], f[j] = o, o += t;
        const int shr = i * 8;
        for (size_t i = 0; i < n; i++) {
            const uint32_t x = a[i];
            b[f[(uint8_t)(x >> shr)]++] = x;
        }
        tmp = a, a = b, b = tmp;
    }
}

static void SIMD(uint32_t *const A,
                 const size_t n,
                 uint32_t *const B) {

    if (n < 2) return;

    const size_t n16 = n / 16;
    uint32_t *pa = A, *pb = B, *tmp;

    _Alignas(64) uint32_t _f[16], _aidx[16];
    for (size_t i = 0; i < 16; i++) _f[i] = i * 256, _aidx[i] = i * n16;

    const __m512i m255 = _mm512_set1_epi32(255),
                   one = _mm512_set1_epi32(1),
                    fb = _mm512_load_si512(_f),
                  aidx = _mm512_load_si512(_aidx);

    for (size_t i = 0; i < 4; i++) {

        const int shr = i * 8;
        _Alignas(64) uint32_t F[16][256] = {};

        for (uint32_t *p = pa, *const P = p + n16; p < P; p++) {

#define GET_A_FIDX \
            const __m512i a = _mm512_i32gather_epi32(aidx, p, 4), \
                       fidx = _mm512_add_epi32( \
                fb, _mm512_and_si512(m255, _mm512_srli_epi32(a, shr)) \
            )

            GET_A_FIDX;
            _mm512_i32scatter_epi32(
                F, fidx, _mm512_add_epi32(one, _mm512_i32gather_epi32(fidx, F, 4)), 4
            );
        }

        for (size_t j = 0, o = 0; j < 256; j++) {
            for (size_t i = 0, t; i < 16; i++)
                t = F[i][j], F[i][j] = o, o += t;
        }

        for (uint32_t *p = pa, *const P = p + n16; p < P; p++) {
            GET_A_FIDX;
            const __m512i bidx = _mm512_i32gather_epi32(fidx, F, 4);
            _mm512_i32scatter_epi32(F, fidx, _mm512_add_epi32(one, bidx), 4);
            _mm512_i32scatter_epi32(pb, bidx, a, 4);
        }

        tmp = pa, pa = pb, pb = tmp;
    }
}

static int qcmp(const void *_a, const void *_b) {
    const uint32_t *a = _a, *b = _b;
    return (*a > *b) - (*a < *b);
}

#define N (size_t)1e6
_Static_assert(N % 16 == 0);

static uint32_t R[N], S[N], A[N], B[N];

int main(void) {

    // srand(time(NULL));
    for (size_t i = 0; i < N; i++) R[i] = S[i] = A[i] = rand();

    {
        volatile int32_t ramp = 1e9; while (--ramp > 0);
    }

    struct timespec ___t1, ___t2;
    clock_gettime(CLOCK_REALTIME, &___t1);
    Scalar(S, N, B);
    clock_gettime(CLOCK_REALTIME, &___t2);
    const int64_t ts = TIME_DIFF_MS;

    clock_gettime(CLOCK_REALTIME, &___t1);
    SIMD(A, N, B);
    clock_gettime(CLOCK_REALTIME, &___t2);

    qsort(R, N, sizeof(uint32_t), qcmp);
    _(memcmp(R, S, N * sizeof(uint32_t)))
    _(memcmp(R, A, N * sizeof(uint32_t)))

    printf(
        "\n"
        "\tScalar: %10ld ms\n"
        "\tSIMD:   %10ld ms\n"
        "\n"
    , ts, TIME_DIFF_MS);

    return 0;
}
