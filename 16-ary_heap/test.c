#include <stdio.h>
#include "heap.h"

#define _(CND) { \
    if (CND) \
        fprintf(stderr, "🤔, line: %d\n", __LINE__); \
        return 1; \
    }

#define N (size_t)1e6

int main(void) {

    minq_t q;
    _(minq_init(&q, N))

    minq_release(&q);

    return 0;
}
