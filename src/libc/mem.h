#ifndef MEM_H
#define MEM_H

// Initialize the memory allocator (sets up the initial free block)
void mem_init(void);

// Allocate size bytes of memory on the heap
void* malloc(unsigned int size);

// Free a previously allocated memory block
void free(void* ptr);

#endif
