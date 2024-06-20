#include <immintrin.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define TIME_DIFF_NS   ((___t2.tv_nsec + ___t2.tv_sec * (int64_t)1e9) - \
                        (___t1.tv_nsec + ___t1.tv_sec * (int64_t)1e9))

#define unlikely(x) __builtin_expect(!!(x), 0)
#define _(CND) \
    if (unlikely(CND)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        exit(1); \
    }

#define SAFE_INTEGER (((uint64_t)1 << 53) - 1)

static uint64_t number_of_divisors(uint64_t x) {

    _(x == 0)

    uint64_t res = __builtin_ctzll(x), c = 1;
    x >>= res, ++res;

    while (x % 3 == 0) x /= 3, ++c;
    res *= c, c = 1;
    while (x % 5 == 0) x /= 5, ++c;
    res *= c, c = 1;

    _(x > SAFE_INTEGER)

    const __m256d delta = _mm256_set1_pd(30.0);

    __m256d t = _mm256_set1_pd((double)x),
           da = _mm256_set_pd( 7.0, 11.0, 13.0, 17.0),
           db = _mm256_set_pd(19.0, 23.0, 29.0, 31.0), ea, eb;

    __m256i ma, mb;

    uint64_t S = sqrt(x), L = 31;
    const size_t offsets[] = {24, 20, 18, 14, 12, 8, 2, 0};

    while (L <= S) {

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

        if (unlikely(_mm256_movemask_epi8(_mm256_or_si256(ma, mb)) != 0)) {
            for (size_t i = 0; i < 8; i++) {
                const uint64_t d = L - offsets[i];
                while (x % d == 0) x /= d, ++c;
                res *= c, c = 1;
            }
            S = sqrt(x), t = _mm256_set1_pd((double)x);
        }

        da = _mm256_add_pd(da, delta),
        db = _mm256_add_pd(db, delta), L += 30;
    }

    for (L -= 24; L <= S; L += 2) {
        while (x % L == 0) x /= L, ++c;
        res *= c, c = 1;
    }

    return x > 1 ? res * 2 : res;
}

int main(void) {

    {
        volatile int32_t ramp = 1e9; while (--ramp > 0);
    }

    uint64_t r, ns;

    struct timespec ___t1, ___t2;
    clock_gettime(CLOCK_REALTIME, &___t1);
    r = number_of_divisors(SAFE_INTEGER);
    clock_gettime(CLOCK_REALTIME, &___t2), ns = TIME_DIFF_NS;

    _(r != 8)

    clock_gettime(CLOCK_REALTIME, &___t1);
    r = number_of_divisors(SAFE_INTEGER - 110);
    clock_gettime(CLOCK_REALTIME, &___t2);

    _(r != 2)

    printf(
        "\n"
        "\t2 ** 53 -   1: %9lu ns\n"
        "\t2 ** 53 - 111: %9ld ns\n"
        "\n"
    , ns, TIME_DIFF_NS);

    return 0;
}
