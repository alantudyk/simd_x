#include <stdio.h>
#include "heap.h"

#define N (size_t)1e6

int main(void) {

    minq_t q;
    if (minq_init(&q, N)) {
        fprintf(stderr, "%d\n", __LINE__);
        return 1;
    }

    minq_release(&q);

    return 0;
}
