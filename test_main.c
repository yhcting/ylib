#include <stdio.h>
#include <malloc.h>

#include "ydef.h"

#ifdef _YDBG

static int  _mem_count = 0;

void* dmalloc(unsigned int sz) {
    _mem_count++;
    return malloc(sz);
}

void dfree(void* p) {
    _mem_count--;
    free(p);
}

int dmem_count() { return _mem_count; }


#define DECL_FUNC(x)    extern void x ();
#include "test_funcs.h"
#undef DECL_FUNC

#define DECL_FUNC(x)    x,
static void(*_test_funcs[])() = {
#include "test_funcs.h"
};
#undef DECL_FUNC


int main() {
    int i;
    for(i=0; i<sizeof(_test_funcs)/sizeof(_test_funcs[0]); i++) {
        (*_test_funcs[i])();
    }

    printf(">>>>>> TEST SUCCESS <<<<<<<\n");
    return 0;
}

#endif /* _YDBG */
