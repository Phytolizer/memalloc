#include "memalloc.h"

#include <tau/tau.h>

TAU_MAIN()

TEST(memalloc, too_big) {
    void* ptr = memalloc(SIZE_MAX);
    CHECK_NULL(ptr);
}
