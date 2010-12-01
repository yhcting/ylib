#include "yqueue.h"
#include "common.h"

#ifdef _YDBG

#include <assert.h>

extern int dmem_count();

void test_queue() {
    int* pi;
    int sv = dmem_count();
    struct yqueue* q = yqueue_create(NULL);

    pi = (int*)ymalloc(sizeof(int));
    *pi = 0;
    yqueue_en(q, pi);
    yassert(1 == yqueue_size(q));
    pi = (int*)ymalloc(sizeof(int));
    *pi = 1;
    yqueue_en(q, pi);
    yassert(2 == yqueue_size(q));

    pi = yqueue_de(q);
    yassert(0 == *pi);
    yassert(1 == yqueue_size(q));
    yfree(pi);

    yqueue_destroy(q);
    yassert(sv == dmem_count());
}

#endif /* _YDBG */
