#include <immintrin.h>
#include <stdlib.h>
#include "heap.h"

bool minq_init(minq_t *const q, size_t c) {
    if ((q->_c = c) == 0)
        return true;
    c = (c + 14) / 16 + 1;
    if ((q->_a = aligned_alloc(64, c * 64)) == NULL)
        return true;
    q->_a += 15, q->_z = 0;
    const __m256i s = _mm256_set1_epi32(INT32_MAX);
    for (int32_t *p = q->_a + 1, *const P = p + (q->_c - 1); p < P; p += 16)
        _mm256_store_si256((void *)p, s),
        _mm256_store_si256((void *)(p + 8), s);
    return false;
}

void minq_release(minq_t *const q) { free(q->_a - 15), *q = (minq_t){}; }

bool minq_push(minq_t *q, const int32_t x) {
    size_t i = q->_z, p;
    if (i == q->_c) return true;
    int32_t *a = q->_a;
    while (i > 0) {
        p = (i - 1) / 16;
        if (a[p] <= x) break;
        a[i] = a[p], i = p;
    }
    a[i] = x, q->_z++;
    return false;
}

bool minq_pop(minq_t *const q, int32_t *const _x) {

    if (q->_z == 0) return true;
    int32_t *const _a = q->_a;
    *_x = _a[0];
    const size_t z = (q->_z -= 1);
    if (z == 0) return false;
    const int32_t sinking_key = _a[z];
    _a[z] = INT32_MAX;
    size_t i = 0, c;

    const __m256i P4 = _mm256_set1_epi32(4);

    while ((c = i * 16 + 1) < z) {

        const __m256i a = _mm256_load_si256((void *)(_a + c)),
                      b = _mm256_load_si256((void *)(_a + c + 8));

        __m256i m = _mm256_min_epi32(a, b);

        m = _mm256_min_epi32(m, _mm256_bsrli_epi128(m, 4));
        m = _mm256_min_epi32(m, _mm256_bsrli_epi128(m, 8));
        m = _mm256_min_epi32(
            _mm256_broadcastd_epi32(_mm256_castsi256_si128(m)),
            _mm256_permutevar8x32_epi32(m, P4)
        );

        const int32_t v = _mm256_extract_epi32(m, 0);
        if (sinking_key <= v) break;

        uint32_t lo = _mm256_movemask_epi8(_mm256_cmpeq_epi32(a, m));
        uint32_t hi = _mm256_movemask_epi8(_mm256_cmpeq_epi32(b, m));
        c += __builtin_ctzll(((uint64_t)hi << 32) | lo) / 4;

        _a[i] = v, i = c;
    }

    _a[i] = sinking_key;

    return false;
}
