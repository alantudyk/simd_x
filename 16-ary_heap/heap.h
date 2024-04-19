#include <inttypes.h>
#include <stdbool.h>

typedef struct minq_t {
    int32_t * _a;
    size_t _z, _c;
} minq_t;

__attribute__((warn_unused_result))
bool minq_init(minq_t *q, size_t c);

void minq_release(minq_t *q);

__attribute__((warn_unused_result))
bool minq_push(minq_t *q, int32_t x);

__attribute__((warn_unused_result))
bool minq_pop(minq_t *q, int32_t *_x);
