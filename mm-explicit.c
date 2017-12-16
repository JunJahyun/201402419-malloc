/*
 * mm-explicit.c - an empty malloc package
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 *
 * @id : 201402419
 * @name : 전 자 현
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {
	
	// Request memory for the initial empty heap
	if(h_ptr = mem_sbrk(DSIZE + 4 * HDRSIZE)) == NULL)
		return -1;
	heap_start = h_ptr;

	PUT(h_ptr, NULL);
	PUT(h_ptr + WSIZE, NULL);
	PUT(h_ptr + DSIZE, 0);
	PUT(h_ptr + DSIZE + HDRSIZE, PACK(OVERHEAD,1));
	PUT(h_ptr + DSIZE + HDRSIZE + FTRSIZE, PACK(OVERHEAD, 1));
	PUT(h_ptr + DSIZE + 2 * HDRSIZE + FTRSIZE, PACK(0,1));

	// Move heap pointer over to footer
	h_ptr += DSIZE + DSIZE;

	// Leave room for the previous and next pointers, place epilogue 3 words down
	epilogue = h_ptr + HDRSIZE;

	// Extend the empty heap with a free block of CHUNKSIZE bytes
	if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
		return -1;


	return 0;
}

inline void *extend_heap(size_t words){
	unsigned *old_epilogue;
	char *bp;
	unsigned size;
	
	// Allocate an even number of words to maintain alignment
	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

	// Request more memory from heap
	if((long)(bp = mem_sbrk(size)) < 0)
		return NULL;

	// Save the old epilogue pointer
	old_epilogue = epilogue;
	epilogue = bp + size - HDRSIZE;

	// Write in the header, footer, and new epilogue
	PUT(HDRP(bp), PACK(size,0));
	PUT(FTRP(bp), PACK(size,0));
	PUT(epilogue, PACK(0,1));

	return coalesce(bp);
}


/*
 * malloc
 */
void *malloc (size_t size) {

	char *bp;				// Block pointer, points to first byte of payload
	unsigned asize;			// Block size adjusted for alignment and overhead
	unsigned extendsize;	// Amount to extend heap if no fit

	// size가 올바르지 않을 때 예외처리 (구현)
	
	// block의 크기 결정 (구현)

	// 결정한 크기에 알맞은 블록을 list에서 검색하여 해당 위치에 할당
	if((bp = find_fit(asize)) != NULL){
		place(bp, asize);
		return bp;
	}

	//free list에서 적절한 블록을 찾지 못했으면 힙을 늘려서 할당
	extendsize = MAX(asize, CHUNKSIZE);
	if(bp = extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;

	place(bp, asize);
	return bp;
	return NULL;
}

/*
 * free
 */
void free (void *ptr) {
    if(!ptr) return;
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
    return NULL;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size) {
    return NULL;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const void *p) {
    return p < mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose) {
}
