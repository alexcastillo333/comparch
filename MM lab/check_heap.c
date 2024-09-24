
#include "umalloc.h"
#include "csbrk.h"
#include "stdio.h"
#include "assert.h"


//Place any variables needed here from umalloc.c or csbrk.c as an extern.
extern memory_block_t *free_head;
extern sbrk_block *sbrk_blocks;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 
 * STUDENT TODO:
 * Required to be completed for checkpoint 1:
 *      - Check that pointers in the free list point to valid free blocks. Blocks should be within
          the valid heap addresses: look at csbrk.h for some clues.
 *        They should also be allocated as free.
 *      - Check if any memory_blocks (free and allocated) overlap with each other. Hint: Run
          through the heap sequentially and check that
 *        for some memory_block n, memory_block n+1 has a sensible block_size and is within the 
          valid heap addresses.
 *      - Ensure that each memory_block is aligned. 
 * 
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {


    // Example heap check:
    // Check that all blocks in the free list are marked free and in a valid address.
    memory_block_t *cur = free_head;
    while (cur != NULL) {
        if (check_malloc_output((void *) cur, cur->block_size_alloc) == -1) {
            printf("\nA block was not in the free list was not.\n");
            return -1;
        }
        if (is_allocated(cur)) {
            printf("\nA block in the free list is marked as Allocated.");
            return -1;
        }
        cur = cur->next;  
    }


    sbrk_block * cur_sbrk_blk = sbrk_blocks;
    // Check if any blocks overlap and if blocks are aligned.
    while (cur_sbrk_blk != NULL) {
        memory_block_t * cur_mem_blk = (memory_block_t *) cur_sbrk_blk->sbrk_start;
        while ((uint64_t) cur_mem_blk < cur_sbrk_blk->sbrk_end) {
            // check if cur_mem_blk is a reasonable size
            if (cur_mem_blk->block_size_alloc > 11 * PAGESIZE) {
                printf("\nmemory block too large\n");
                return -1;
            }
            // check if cur_mem_blk is is within valid memory address
            if ((uint64_t) cur_mem_blk < cur_sbrk_blk->sbrk_start || ((uint64_t) cur_mem_blk) + cur_mem_blk->block_size_alloc - 1 > cur_sbrk_blk->sbrk_end) {
                printf("\nmemory block not in valid mem addy\n");
                return -1;
            }
            // check if cur_mem_blk is aligned
            if (((uint64_t) cur_mem_blk) % ALIGNMENT != 0) {
                printf("\nalignment error\n");
                return -1;
            }
            // assign cur_mem_blk's adjacent block to cur_mem_blk 
            cur_mem_blk = (memory_block_t *) ((char*) cur_mem_blk + (cur_mem_blk->block_size_alloc & ~0x1));
        }
        cur_sbrk_blk = cur_sbrk_blk->next;
    } 
    
    return 0;
}