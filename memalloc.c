#include "memalloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define HEAP_MAX (1024 * 1024)
#define ROUNDUP(size, align) (((size) + (align)-1) & ~((align)-1))

typedef struct {
    ptrdiff_t prev;
    size_t size;
    bool allocated;
} heap_block_t;

static void* heap_start(void) {
    static char* start = NULL;

    if (start == NULL) {
        start = sbrk(HEAP_MAX + sizeof(heap_block_t));
        if ((intptr_t)start == -1) {
            perror("sbrk");
            return NULL;
        }
        heap_block_t* block = (heap_block_t*)start;
        block->size = HEAP_MAX;
        block->allocated = false;
        start += sizeof(heap_block_t);
    }
    return start;
}

void* memalloc(size_t size) {
    void* start = heap_start();
    if (start == NULL) {
        return NULL;
    }

    heap_block_t* header = (heap_block_t*)start - 1;

    while (true) {
        if (header->allocated) {
            header = (heap_block_t*)((char*)header + header->size);
        } else {
            if (header->size - sizeof(heap_block_t) >= size) {
                heap_block_t* block = (heap_block_t*)((char*)header + size);
                block->size = header->size - size - sizeof(heap_block_t);
                block->allocated = false;
                block->prev = block - header;
                header->size = size;
                header->allocated = true;
                return (char*)header + sizeof(heap_block_t);
            }
            header = (heap_block_t*)((char*)header + header->size);
        }
        if ((uintptr_t)((char*)header - (char*)start) >= HEAP_MAX - sizeof(heap_block_t)) {
            return NULL;
        }
    }
}

void memfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    heap_block_t* header = (heap_block_t*)ptr - 1;
    header->allocated = false;
}
