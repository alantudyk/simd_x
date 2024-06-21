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
           da = _mm256_setr_pd( 7.0, 11.0, 13.0, 17.0),
           db = _mm256_setr_pd(19.0, 23.0, 29.0, 31.0);

    uint64_t S = sqrt(x), L = 31;
    const size_t offsets[] = {24, 20, 18, 14, 12, 8, 2, 0};

    for (; L <= S; L += 30) {

#define GET_MASK(a) \
        const uint32_t m##a = _mm256_movemask_epi8( \
            _mm256_cmpeq_epi64( \
                _mm256_castpd_si256(t), \
                _mm256_castpd_si256( \
                    _mm256_mul_pd(_mm256_floor_pd(_mm256_div_pd(t, d##a)), d##a) \
                ) \
            ) \
        ); \
        d##a = _mm256_add_pd(d##a, delta);

        GET_MASK(a)
        GET_MASK(b)

        if (unlikely((ma | mb) != 0)) {
            uint64_t m = ((uint64_t)mb << 32) | ma, i = 0;
            while (m > 0 & i < 8) {
                const uint64_t tz = __builtin_ctzll(m),
                                d = L - offsets[i += tz / 8];
                while (x % d == 0) x /= d, ++c;
                res *= c, c = 1, ++i, m >>= tz + 8;
            }
            S = sqrt(x), t = _mm256_set1_pd((double)x);
        }
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
