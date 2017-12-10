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

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define OVERHEAD 8

#define MAX(x,y) ((x) > (y) ? (x) : (y))

//크기와 할당된 비트를 word로 묶음
#define PACK(size, alloc) ((size) | (alloc))

//주소 p에서 word 읽기, 쓰기
#define GET(p) (*(size_t *)(p))
#define PUT(p,val) (*(size_t *)(p) = (val))

//주소 p에서 크기와 할당 된 필드를 읽음
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

//ptr bp가 주어질 때 주소 계산
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))



static char *heap_listp = 0;
static char *next_bp;

static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);


/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {
    
	if((heap_listp = mem_sbrk(4 * WSIZE)) == NULL)
		return -1;

	PUT(heap_listp, 0);
	PUT(heap_listp + WSIZE, PACK(OVERHEAD, 1));
	PUT(heap_listp + DSIZE, PACK(OVERHEAD, 1));
	PUT(heap_listp + WSIZE + DSIZE, PACK(0, 1));
	heap_listp += 4*WSIZE;

	next_bp = heap_listp;

	if((extend_heap(CHUNKSIZE / WSIZE)) == NULL)
		return -1;

	return 0;
}

static void *extend_heap(size_t words){
	char *bp;
	size_t size;

	size = (words % 2)?(words + 1) * WSIZE : words * WSIZE;
	
	if((long)(bp = mem_sbrk(size)) == -1){
		return NULL;
	}

	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));

	return coalesce(bp);

}




/*
 * malloc
 */
void *malloc (size_t size) {
	char *bp;
	size_t sizeA, sizeB;

	if(size == 0){
		return NULL;
	}

	if(size <= DSIZE){
		sizeA = 2*DSIZE;
	}
	else{
		sizeA = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
	}

	if((bp = find_fit(sizeA)) != NULL){
		
		place(bp, sizeA);
		return bp;
	}

	sizeB = MAX(sizeA, CHUNKSIZE);
	if((bp = extend_heap(sizeB/WSIZE)) == NULL){
		return NULL;
	}
	
	place(bp, sizeA);
	
	return bp;

}


static void *find_fit(size_t asize){
	char* bp;
	bp = next_bp;


	while(GET_SIZE(HDRP(bp)) > 0){
		if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
			next_bp = bp;
			return bp;
		}

		bp = NEXT_BLKP(bp);
	}

	return NULL;

}

static void place(void *bp, size_t asize){
	
	size_t bsize = GET_SIZE(HDRP(bp));

	if((bsize - asize) >= 2 * DSIZE){
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));

		bp = NEXT_BLKP(bp);

		PUT(HDRP(bp), PACK(bsize - asize, 0));
		PUT(FTRP(bp), PACK(bsize - asize, 0));
	}
	else{
		PUT(HDRP(bp), PACK(bsize, 1));
		PUT(FTRP(bp), PACK(bsize, 1));
	}

}



/*
 * free
 */
void free (void *ptr) {
    if(ptr == 0) return;
	size_t size = GET_SIZE(HDRP (ptr));

	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));

	next_bp = coalesce(ptr);
}

static void *coalesce(void *bp){
	
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	//이전 블럭의 할당 여부 0 = NO, 1 = YES

	size_t next_alloc = GET_ALLOC(HDRP (NEXT_BLKP(bp)));
	//다음 블럭의 할당 여부 0 = NO, 1 = YEs

	size_t size = GET_SIZE(HDRP(bp));
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
	
	else if(next_alloc && !prev_alloc){

		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		
		bp = PREV_BLKP(bp);
	}



	//case4 : 이전 블럭 최하위 bit가 0이고 (비할당), 다음 블럭 최하위 bit가 
	//        0인경우(비할당) 이전 블럭, 현재 블럭, 다음 블럭을 모두 병합한
	//		  뒤 새로운 bp return

	else{
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	
	}

	return bp;
	//병합된 블럭의 주소 bp return

}



/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
 	size_t oldsize;
	void *newptr;

	//if size == 0 then this is just free, and we return NULL.
	if(size == 0){
		free(oldptr);
		return 0;
	}
	
	//if oldptr is NULL, then this is just malloc.
	if(oldptr == NULL){
		return malloc(size);
	}

	newptr = malloc(size);

	//if realloc() fails the original block is left untouched
	if(!newptr){
		return 0;
	}

	
	// Copy the old data.
	oldsize = *SIZE_PTR(oldptr);
	if(size < oldsize) oldsize = size;
	memcpy(newptr, oldptr, oldsize);

	// Free the old block.
	free(oldptr);

	return newptr;

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
