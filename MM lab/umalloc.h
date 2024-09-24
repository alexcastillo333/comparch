#include <stdlib.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bits 1-3 are unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    size_t block_size_alloc;
    struct memory_block_struct *next;
} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c

/*
*  STUDENT TODO:
*      Write 1-2 sentences for each function explaining what it does. Don't just repeat the name 
       of the function back to us.
*/

// Returns true if the block pointed to by block is allocated. An allocated block's header will 
// have a '1' as the least significant bit.
bool is_allocated(memory_block_t *block);

// Marks a block as allocated. The least significant bit of the blocks header will be set to '1'.
void allocate(memory_block_t *block);

// Marks a block as free. The least significant bit of the blocks header will be set to '0'.
void deallocate(memory_block_t *block);

// Returns the size of a block which includes the header, payload, and any padding.
size_t get_size(memory_block_t *block);

// Returns the a pointer to the next free block in the free list after block.
memory_block_t *get_next(memory_block_t *block);

// Inserts a block struct into the location that block points to. Size must be 16 byte aligned,
// alloc must be '0' or '1' and block can not be null.
void put_block(memory_block_t *block, size_t size, bool alloc);


// Returns a pointer to the beginning of the payload. The beginning of the payload is 16 bytes 
// after start of the block (due to the header). 
void *get_payload(memory_block_t *block);

// Returns a pointer to the block that the payload was in. The pointer will be 16 bytes before the
// start of payload.
memory_block_t *get_block(void *payload);


// Wrapper for coalesce to implement deffered coalescing.
void coalesce_all();


//week 1

// Iterates through the address ordered free list until the first block with a payload of
// at least size bytes is found. If the block will be split if it has at least 64 bytes leftover
// after marking the block to be allocated.
memory_block_t *find(size_t size);

// Calls csbrk to extend the size of the heap.
memory_block_t *extend(size_t size);

// Splits block into an allocated block with size size and a free block with size
// block->block_size_alloc - size.
memory_block_t *split(memory_block_t *block, size_t size);

// Returns a free block that is the size of block plus the size of block->next when both
// blocks are free.
memory_block_t *coalesce(memory_block_t *block);


// Portion that may not be edited
// week 1

// Initializes a free block with size  = 11 * PAGESIZE by calling csbrk.
int uinit();
// Returns a pointer to a payload with at least size bytes.
void *umalloc(size_t size);
// Marks the block whose payload is equal to ptr as free and inserts it into free list.
void ufree(void *ptr);