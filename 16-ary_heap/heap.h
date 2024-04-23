#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef struct minq_t {
    int32_t *_a;
    size_t _z, _c;
} minq_t;

#define CHECK __attribute__((warn_unused_result))

CHECK bool minq_init(minq_t *q, size_t c);
void minq_release(minq_t *q);
CHECK bool minq_push(minq_t *q, int32_t x);
CHECK bool minq_pop(minq_t *q, int32_t *_x);

#undef CHECK
