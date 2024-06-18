#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define TIME_DIFF_NS   ((___t2.tv_nsec + ___t2.tv_sec * (int64_t)1e9) - \
                        (___t1.tv_nsec + ___t1.tv_sec * (int64_t)1e9))
#define TIME_DIFF_MS   (TIME_DIFF_NS / (int64_t)1e6)
#define TIME_DIFF_MCS  (TIME_DIFF_NS / (int64_t)1e3)

#define _(CND) \
    if (__builtin_expect(!!(CND), 0)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        return 1; \
    }

#define SAFE_INTEGER (((uint64_t)1 << 53) - 1)

static uint64_t number_of_divisors(uint64_t x) {

    _(x == 0)

    uint64_t dd = __builtin_ctzll(x), r = 1, fd = 3, ra[4];
    x >>= dd, ++dd;

    while (x % 3 == 0) x /= 3, ++r;
    dd *= r, r = 1;
    while (x % 5 == 0) x /= 5, ++r;
    dd *= r, r = 1;

    _(x > SAFE_INTEGER)

    const __m256d t = _mm256_set1_pd((double)x),
                  p = _mm256_set1_pd(30.0);

    __m256d da = _mm256_set_pd( 7.0, 11.0, 13.0, 17.0),
            db = _mm256_set_pd(19.0, 23.0, 29.0, 31.0), ea, eb;

    const __m256i P = _mm256_set1_epi64x(1);

    __m256i res = _mm256_setzero_si256(), ma, mb;

    const uint64_t S = sqrt(x);

    while (fd + 24 <= S) {

        ea = _mm256_floor_pd(_mm256_div_pd(t, da)),
        eb = _mm256_floor_pd(_mm256_div_pd(t, db));

        ma = _mm256_cmpeq_epi64(
            _mm256_castpd_si256(t),
            _mm256_castpd_si256(_mm256_mul_pd(ea, da))
        ),
        mb = _mm256_cmpeq_epi64(
            _mm256_castpd_si256(t),
            _mm256_castpd_si256(_mm256_mul_pd(eb, db))
        );

        res = _mm256_add_epi64(
            res,
            _mm256_add_epi64(
                _mm256_and_si256(ma, P),
                _mm256_and_si256(mb, P)
            )
        );

        da = _mm256_add_pd(da, p),
        db = _mm256_add_pd(db, p), fd += 30;
    }

    _mm256_storeu_si256((void *)ra, res);
    for (int i = 0; i < 4; i++) r += ra[i];

    while (fd <= S)
        r += x % fd == 0, fd += 2;

    return dd * (r * 2 - (S * S == x));
}

int main(void) {

    {
        volatile int32_t ramp = 1e9; while (--ramp > 0);
    }

    uint64_t r;

    struct timespec ___t1, ___t2;
    clock_gettime(CLOCK_REALTIME, &___t1);
    r = number_of_divisors(SAFE_INTEGER);
    clock_gettime(CLOCK_REALTIME, &___t2);

    _(r != 8)

    printf("\n\t%ld ms\n\n", TIME_DIFF_MS);

    return 0;
}
