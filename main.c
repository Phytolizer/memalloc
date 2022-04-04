#include "memalloc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    puts("hello world");
    printf("%p\n", memalloc(SIZE_MAX));
}
