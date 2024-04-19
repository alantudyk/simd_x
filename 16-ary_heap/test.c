#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "heap.h"

#define TIME_DIFF_NS   ((___t2.tv_nsec + ___t2.tv_sec * (int64_t)1e9) - \
                        (___t1.tv_nsec + ___t1.tv_sec * (int64_t)1e9))
#define TIME_DIFF_MS   (TIME_DIFF_NS / (int64_t)1e6)
#define TIME_DIFF_MCS  (TIME_DIFF_NS / (int64_t)1e3)

#define _(CND) \
    if (__builtin_expect(!!(CND), 0)) { \
        fprintf(stderr, "\n\t🤔, line: %d\n\n", __LINE__); \
        return 1; \
    }

static int cmp(const void *_a, const void *_b) {
    const int32_t *a = _a, *b = _b;
    return (*a > *b) - (*a < *b);
}

#define N (size_t)1e6
static int32_t R[N], A[N];

int main(void) {

    // srand(time(NULL));
    for (size_t i = 0; i < N; i++) R[i] = rand();

    {
        volatile int32_t ramp = 1e9; while (--ramp > 0);
    }

    minq_t q;

    struct timespec ___t1, ___t2;
    clock_gettime(CLOCK_REALTIME, &___t1);

    _(minq_init(&q, N))
    for (size_t i = 0; i < N; i++) { bool _ = minq_push(&q, R[i]); }

    minq_release(&q);

    clock_gettime(CLOCK_REALTIME, &___t2);
    printf("\n\tHeap-sort (w/o `heapify()`): %ld ms\n", TIME_DIFF_MS);

    clock_gettime(CLOCK_REALTIME, &___t1);
    qsort(R, N, 4, cmp);
    clock_gettime(CLOCK_REALTIME, &___t2);
    printf("\n\t`qsort()`: %ld ms\n\n", TIME_DIFF_MS);

    _(!!memcmp(R, A, N * sizeof(int32_t)))

    return 0;
}
