#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Alejandro Castillo alc5938" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

// A sample pointer to the start of the free list.
memory_block_t *free_head;

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block, which includes the header, payload, and any padding.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block in the free list.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 *  STUDENT TODO:
 *      Describe how you select which free block to allocate. What placement strategy are you using?

        The free list will be in address order. The first block that is greater than or 
        equal to the requested size in this list will be selected for allocation. 
        If no block is large enough, we coalesce then run through the list again. We extend and 
        run through the list if coalescing did not make a block large enough.
 */

/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    //? STUDENT TODO

    for (int i = 0; i < 3; i++)
    {

    memory_block_t * temp = free_head->next;
    memory_block_t * trailer = free_head;

    // Check if the free_head satisfies the request
    if (trailer->block_size_alloc >= size) {
        // Split free_head if there will be at least 32 free bytes after allocating size bytes.
        if (trailer->block_size_alloc - size >= ALIGNMENT * 2) {
            return split(trailer, size);
        }
        free_head = temp;
        // In this case, we have allocated the free_head, so we need to extend
        if (free_head == NULL) {
            free_head = extend(0);
            free_head->block_size_alloc = 11 * PAGESIZE;
        }
        allocate(trailer);
        return trailer;
    }

    // Continue checking if any block in the free list satisfies the request.
    while (temp != NULL) {
        if (temp->block_size_alloc >= size) {
            if (temp->block_size_alloc - size >= ALIGNMENT * 2) {
                return split(temp, size);
            }
            // in this case the size of temp is large enough to satisfy malloc but not large 
            // enough to split
            trailer->next = temp->next;
            allocate(temp);
            return temp;
        }
        temp = temp->next;
        trailer = trailer->next;
    }
    // In this case, no blocks in the free list were big enough during the first loop, so we
    // coalesce
    if (i < 1) {
        coalesce_all();
    // In this case, we have already coalesced but there was still no block large enough, so we
    // extend.
    } else {
        temp = extend(0);
        temp->block_size_alloc = PAGESIZE * 11;
        trailer->next = temp;
    }
    }
    return NULL;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    //? STUDENT TODO
    return (memory_block_t *) csbrk(PAGESIZE * 11);
}

/*
 *  STUDENT TODO:
 *      Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?

        A block will be split if the difference between size of the original block and the size
        of the request is greater than or equal to 32.

        When split, the free block's header will be at the same location as the original block's 
        header. The allocated block's header will be immediately after the free block.

        Split returns a pointer to the allocated block.
*/

/*
 * split - splits a given block in parts, one allocated, one free. Returns the allocated block.
 */


memory_block_t *split(memory_block_t *block, size_t size) {
    //? STUDENT TODO

    int alloc_start = block->block_size_alloc - size;
    // update size of free block
    block->block_size_alloc = alloc_start;
    deallocate(block);
    // pointer of alloced_block is alloc_start bytes past the pointer of block
    memory_block_t * alloced_block = (memory_block_t *) ((char *) block + alloc_start);
    put_block(alloced_block, size, 1);
    return alloced_block;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    //? STUDENT TODO fix this tomorrow
    // if coalesce was called, it must be the case that block's next reference
    // is adjacent to block (because of address ordering of free list).
    block->block_size_alloc += block->next->block_size_alloc;
    block->next = block->next->next;
    deallocate(block);
    return block;
}

// coalesces all possible free blocks
void coalesce_all() {
    memory_block_t * temp = free_head;
    while (temp != NULL) {
        // no more free blocks to coalesce
        if (temp->next == NULL) {
            return;
        }


        // coalesce if two free blocks are adjacent.
        if ((uint64_t) temp->next == (uint64_t) (temp) + temp->block_size_alloc) {
            temp = coalesce(temp);
        } else {
            temp = temp->next;
        }
    }
}



/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */


int uinit() {

    // Increase the size of the heap, returning a pointer to the start of the new block
    free_head = (memory_block_t *) csbrk(PAGESIZE * 11);
    free_head->next = NULL;
    free_head->block_size_alloc = PAGESIZE * 11;
    deallocate(free_head);
    //* STUDENT TODO
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {

    if (size == 0) {
        return NULL;
    }

    // calls to find will always pass size as a multiple of 16 greater than or equal to 32.
    int reqSize;
    if (size > ALIGNMENT) {
        if (size % ALIGNMENT != 0) {
        reqSize = size + 2 * ALIGNMENT - size % ALIGNMENT;
        } else {
            reqSize = size + ALIGNMENT;
        }
    } else {
        reqSize = ALIGNMENT * 2;
    }

    memory_block_t * block = find(reqSize);
    block->next = NULL;
    void * out = get_payload(block);
    check_malloc_output(out, reqSize - ALIGNMENT);
    return out;
}

/*
 *  STUDENT TODO:
 *      Describe your free block insertion policy.
        A free block will be inserted into the free list by address.
        Each block after a block b will have a higher address than b and each block before b will have a
        smaller address.
*/


/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc. Inserts the newly freed block into the correct
   location in the free list.
 */
void ufree(void *ptr) {
    //* STUDENT TODO

    memory_block_t * block = get_block(ptr);
    deallocate(block);
    // Insert block as the free_head if its memory address is less than free_head's address.
    if ((uint64_t) block < (uint64_t) free_head) {
        block->next = free_head;
        free_head = block;
        return;
    }

    memory_block_t * temp = free_head->next;
    memory_block_t * free_trailer = free_head;

    // Insert the block in the correct position in the address based free list.
    while (temp != NULL) {
        if ((uint64_t) block > (uint64_t) free_trailer && (uint64_t) block < (uint64_t) temp) {
            free_trailer->next = block;
            block->next = temp;
            return;
        }
        temp = temp->next;
        free_trailer = free_trailer->next;
    }

    // free_head->next was null,so this is where block goes.
    free_head->next = block;

    return;
}