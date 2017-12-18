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

#define HDRSIZE 4
#define FTRSIZE 4
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define OVERHEAD

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#define PACK(size, alloc) ((unsigned) ((size) | (alloc)))

#define GET(p) (*(unsigned *)(p))
#define PUT(p, val) (*(unsigned *)(p) = (unsigned)(val))
#define GET8(p) (*(unsigned long *)(p))
#define PUT8(p, val) (*(unsigned long *)(p) = (unsigned long)(val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char*)(bp) - DSIZE))

#define NEXT_FREEP(bp) ((char *)(bp))
#define PREV_FREEP(bp) ((char *)(bp) + WSIZE)

#define NEXT_FREE_BLKP(bp) ((char *)GET8((char *)(bp)))
#define PREV_FREE_BLKP(bp) ((char *)GET8((char *)(bp) + WSIZE))

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
	
	char *bp;
	char *oldp;
	size_t size;


	// Allocate an even number of words to maintain alignment
	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

	// Request more memory from heap
	if((long)(bp = mem_sbrk(size)) < 0)
		return NULL;

	PUT(HDRP(bp), PACK(size,0));		//확장 후 헤더
	PUT(FTRP(bp), PACK(size,0));		//확장 후 푸터
	PUT(NEXT_FREEP(bp), NEXT_BLKP(bp)); //다음 블록 에필로그

	for(oldp = h_ptr; NEXT_FREE_BLKP(oldp) != bp; oldp = NEXT_FREE_BLKP(oldp)){
	PUT(PREV_FREEP(bp), oldp);
	}
	
	bp = coalesce(bp);

	return bp;
}


/*
 * malloc
 */
void *malloc (size_t size) {

	char *bp;				// Block pointer, points to first byte of payload
	unsigned asize;			// Block size adjusted for alignment and overhead
	unsigned extendsize;	// Amount to extend heap if no fit

	if(size <= 0) {		
		return NULL;
	}
	
	if(h_ptr == 0){
		mm_init();
	}

	asize = ALIGN(size + OVERHEAD);

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

static void *find_fit(size_t asize){
}

static void place(void *bp, size_t asize){
	
	size_t bsize = GET_SIZE(HDRP(bp));
	void *prevP = PREV_FREE_BLKP(bp);
	void *nextP = NEXT_FREE_BLKP(bp);

	if((bsize - asize) >= (2*DSIZE)){
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));

		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK(bsize - asize, 0));
		PUT(FTRP(bp), PACK(bsize - asize, 0));
		PUT(NEXT_FREEP(prevP), bp);
		PUT(PREV_FREEP(bp), prevP);
		PUT(NEXT_FREEP(bp), nextP);
	}
	else{
		PUT(HDRP(bp), PACK(bsize, 1));
		PUT(FTRP(bp), PACK(bsize, 1));
		PUT(NEXT_FREEP(prevP), nextP);
	}
}



/*
 * free
 */
void free (void *ptr) {
    if(!ptr) return;
	size_t size = GET_SIZE(HDRP (ptr));
	
	if(h_ptr == 0){
		mm_init();
	}

	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	coalesce(ptr);
}

static void *coalesce(void *bp){
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	if(prev_alloc && next_alloc){
		return bp;
	}
		
	if(!next_alloc){
		size = size + GET_SIZE(HDRP(NEXT_BLKP(bp)));
	
		PUT(NEXT_FREEP(bp), NEXT_FREE_BLKP(NEXT_BLKP(bp)));
		PUT(PREV_FREEP(NEXT_FREE_BLKP(bp)), bp);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	
	if(!prev_alloc) {
		size = size + GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(NEXT_FREEP(PREV_BLKP(bp)), NEXT_FREE_BLKP(bp));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));

		bp = PREV_BLKP(bp);
		PUT(PREV_FREEP(NEXT_FREE_BLKP(bp)), bp);

	}

	return bp;

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
