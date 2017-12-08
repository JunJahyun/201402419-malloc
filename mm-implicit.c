/*
 * mm-implicit.c - an empty malloc package
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
    
	if((heap_listp = mem_sbrk(4 * WSIZE)) == NULL)
		return -1;

	PUT(heap_listp, 0);
	PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1));
	PUT(heap_listp + DSIZE, PACK(OVERHEAD, 1));
	PUT(heap_listp + WSIZE + DIZE, PACK(0, 1));
	heap_listp += DSIZE;

	if((extend_heap(CHUNKSIZE / WSIZE)) == NULL)
		return -1;

	return 0;
}

/*
 * malloc
 */
void *malloc (size_t size) {
    return NULL;
}

/*
 * free
 */
void free (void *ptr) {
    if(ptr == 0) return;
	size_t size = GET_SIZE(HDRP (ptr));

	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));

	coalesce(ptr);
}

void *coalesce(void *bp){
	
	size_t prev alloc = GET ALLOC(FTRP(PREV_BLKP(bp)));
	//이전 블럭의 할당 여부 0 = NO, 1 = YES

	size_t next alloc = GET ALLOC(HDRP (NEXT_BLKP(bp)));
	//다음 블럭의 할당 여부 0 = NO, 1 = YEs

	size_t size = GET SIZE(HDRP(bp));
	//현재 블럭의 크기


	//case 1 : 이전 블럭, 다음 블럭 최하위 bit가 둘다 1 인 경우 (할당)
	// 		   다음 블럭과 병합한 뒤 bp return

	if(prev_alloc && next_alloc){
		return bp;
	}

	//case2 : 이전 블럭 최하위 bit가 1이고 (할당), 다음 블럭 최하위 bit가 0
	//		  인 경우(비할당) 다음 블럭과 병합한 뒤 bp return

	else if(prev_alloc && !next_alloc){
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}


	//case3 : 이전 블럭 최하위 bit가 0이고 (비할당), 다음 블럭 최하위 bit가 
	//	      1인 경우(할당) 이전 블럭과 병합한 뒤 새로운 bp return
	
	else if(){
	}


	//case4 : 이전 블럭 최하위 bit가 0이고 (비할당), 다음 블럭 최하위 bit가 
	//        0인경우(비할당) 이전 블럭, 현재 블럭, 다음 블럭을 모두 병합한
	//		  뒤 새로운 bp return

	else{
	}

	return bp;
	//병합된 블럭의 주소 bp return

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
