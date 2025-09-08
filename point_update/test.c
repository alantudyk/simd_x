#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

static int64_t a[(int)1e6];

int main(void) {
    int n, q, _, i, t;
    _ = scanf("%d %d", &n, &q);
    for (i = 0; i < n; i++)
        _ = scanf("%ld", a + n + i);
    for (i = n; --i;)
        a[i] = a[i * 2] + a[i * 2 + 1];
    const __m512i z = _mm512_setzero_si512();
    const __m256i
        shr0 = _mm256_set_epi32( 0,  1,  2,  3,  4,  5,  6,  7),
        shr1 = _mm256_set_epi32( 8,  9, 10, 11, 12, 13, 14, 15),
        shr2 = _mm256_set_epi32(16, 17, 18, 19, 20, 21, 22, 23);
    while (q--) {
        _ = scanf("%d", &t);
        if (t == 0) {
            int p, x;
            _ = scanf("%d %d", &p, &x);
            p += n;
            // while (p > 0) a[p] += x, p >>= 1;
#define SHR(N) i##N = _mm256_srlv_epi32(i, shr##N)
            const __m256i i = _mm256_set1_epi32(p), SHR(0), SHR(1), SHR(2);
#define CMPGT(N) \
    m##N = _mm256_cmpgt_epi32_mask(i##N, _mm512_castsi512_si256(z))
            const __mmask8 CMPGT(0), CMPGT(1), CMPGT(2);
#define GATHER(N) r##N = _mm512_mask_i32gather_epi64(z, m##N, i##N, a, 8)
            const __m512i
                d = _mm512_set1_epi64((int64_t)x),
                GATHER(0), GATHER(1), GATHER(2);
#define SCATTER(N) \
    _mm512_mask_i32scatter_epi64(a, m##N, i##N, _mm512_add_epi64(r##N, d), 8)
            SCATTER(0), SCATTER(1), SCATTER(2);
        } else {
            int l, r;
            _ = scanf("%d %d", &l, &r);
            l += n;
            r = (r - 1) + n;
            int64_t s = 0;
            while (l <= r) {
                if ( l & 1) s += a[l++];
                if (~r & 1) s += a[r--];
                l >>= 1;
                r >>= 1;
            }
            printf("%ld\n", s);
        }
    }
    return 0;
}
