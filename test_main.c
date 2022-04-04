#include "memalloc.h"

#include <tau/tau.h>

TAU_MAIN()

TEST(memalloc, too_big) {
    void* ptr = memalloc(SIZE_MAX);
    CHECK_NULL(ptr);
}

TEST(memalloc, ok) {
    void* ptr = memalloc(1);
    CHECK_NOT_NULL(ptr);
    memfree(ptr);
}

TEST(memalloc, loop) {
    for (size_t i = 0; i < 1000000; i++) {
        void* ptr = memalloc(1);
        CHECK_NOT_NULL(ptr);
        memfree(ptr);
    }
}
