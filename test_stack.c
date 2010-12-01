#include "ystack.h"
#include "common.h"

#ifdef _YDBG

#include <assert.h>

extern int dmem_count();

void test_stack() {
    int* pi;
    int sv = dmem_count();
    struct ystack*  s = ystack_create(NULL);

    pi = (int*)ymalloc(sizeof(int));
    *pi = 0;
    ystack_push(s, pi);
    yassert(1 == ystack_size(s));
    pi = (int*)ymalloc(sizeof(int));
    *pi = 1;
    ystack_push(s, pi);
    yassert(2 == ystack_size(s));

    pi = ystack_pop(s);
    yassert(1 == *pi);
    yassert(1 == ystack_size(s));
    yfree(pi);

    ystack_destroy(s);
    yassert(sv == dmem_count());
}

#endif /* _YDBG */
