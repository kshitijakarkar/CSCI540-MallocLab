/* 
     Kshitija Karkar
     Malloc lab - Implemented using binary tree
     Student ID: 006948384

 * Simple allocator based on implicit free lists with boundary 
 * tag coalescing. Each block has header and footer of the form:
 * 
 *      31                     3  2  1  0 
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      ----------------------------------- 
 * 
 * where s are the meaningful size bits and a/f is set 
 * if the block is allocated. The list has the following form:
 *
 * begin                                                          end
 * heap                                                           heap  
 *  -----------------------------------------------------------------   
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *          |       prologue      |                       | epilogue |
 *          |         block       |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */

#include <stdio.h>
#include "mm.h"
#include "memlib.h"
#include <string.h>

team_t team = {
  /* Team name */
  "MyDynamicMemoryImplemenation",
  /* First member's full name */
  "Kshitija_Karkar",
  /* First member's email address */
  "kkarkar@mail.csuchico.edu",
  /* (leave blank) */
  "",
  /* (leave blank) */
  ""
};

/* $begin mallocmacros */
/* Basic constants and macros */
#define CHUNKSIZE  (1<<12)   /* initial heap size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */
#define PADDING     4
#define PROLOGSIZE  16
#define EPILOGSIZE  8



static inline int MAX(int x, int y) {
  return x > y ? x : y;
}

//
// Pack a size and allocated bit into a word
// We mask of the "alloc" field to insure only
// the lower bit is used
//
static inline size_t PACK(size_t size, int alloc) {
  return ((size) | (alloc & 0x1));
}

//
// Read and write a word at address p
//
static inline size_t GET(void *p) { return  *(size_t *)p; }
static inline void PUT( void *p, size_t val)
{
  *((size_t *)p) = val;
}

//
// Read the size and allocated fields from address p
//
static inline size_t GET_SIZE( void *p )  { 
  return GET(p) & ~0x7;
}

static inline int GET_ALLOC( void *p  ) {
  return GET(p) & 0x1;
}

//
// Given block ptr bp, compute address of its header and footer
//
static inline void *HDRP(void *bp) {

  return ( (char *)bp) - WSIZE;
}
static inline void *FTRP(void *bp) {
  return ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE);
}

//
// Given block ptr bp, compute address of next and previous blocks
//
static inline void *NEXT_BLKP(void *bp) {
  return  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)));
}

static inline void* PREV_BLKP(void *bp){
  return  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)));
}

/* global variables pointer to the first block */
static char *heap_listp;
static void *root_node;

/* function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *place(void *bp, size_t asize);
static void *find_fit(void *root,size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void checkblock(void *bp);

/* Binary Tree function declarations */
void *insert(void *root, void *bp);
void *delete(void *root, void *bp);
void *parent_node(void *root, void *bp);
void *inorder_replace(void *bp);
int mm_children(void *root);

/* 
 * mm_init - Initialize the memory manager 
 */
/* $begin mminit */
int mm_init(void) 
{
    void *bp;

    root_node = NULL;

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(PROLOGSIZE)) == NULL)
        return -1;

    PUT(heap_listp, 0);                        /* alignment padding */
    PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));  /* prologue header */ 
    PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));  /* prologue footer */ 
    PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));   /* epilogue header */
    heap_listp += DSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    bp = extend_heap(CHUNKSIZE/WSIZE);

    if (bp == NULL)
        return -1;

    root_node = insert(root_node, bp);

    return 0;
}
/* $end mminit */

/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size) 
{
    size_t asize;      /* adjusted block size */
    size_t extendsize; // amount to extend heap if no fit 
    char *bp;

    // Adjust block size to include overhead and alignment reqs. 
    if (size <= DSIZE)
        asize = OVERHEAD + DSIZE;
    else
        asize = ADJUSTSIZE(size);
    
    /* Search the free list for a fit */
    if ((bp = find_fit(root_node,asize)) != NULL) 
    {
        root_node = delete(root_node,bp); // no block available so delete
        bp = place(bp, asize);
        return bp;
    }
    else
    {
    // Get more memory and place the block 
    	extendsize = MAX(asize,CHUNKSIZE);
    }

    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    else
    {
       bp = place(bp, asize);
    }
    return bp;
} 
/* $end mmmalloc */



/* 
 * find_fit - Find a fit for a block with 'size' bytes 
 */
static void *find_fit(void *root,size_t size)
{
   //find best fit by comparing with root and moving  
   //either left or right
	  	
   //recursive search function of binary tree
   void* bp=NULL;
   if(root == NULL)
   {
     return NULL;
   }
   else
   {        
      int root_size = GETSIZE(root);
    
      if(root_size  ==  size)
      {
         return root;
      }
      else if(root_size > size)
      {
         bp = find_fit(LEFT_CHLD(root), size);
      }
      else if (root_size < size)
      {
         bp = find_fit(RIGHT_CHLD(root), size);
      }
    
      if(bp == NULL)
      {
         if(root_size < size) // If it's too small, return NULL 
            return NULL;
         else 
            return root;
      }
      else 
            return bp;
   }
                    
}



/* 
 * mm_free - Free a block 
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    root_node = insert(root_node,coalesce(bp));
}

/* $end mmfree */


void *mm_realloc(void *ptr, size_t size)
{   

    void *newp;
  size_t copySize;

  newp = mm_malloc(size);
  if (newp == NULL) {
    printf("ERROR: mm_malloc failed in mm_realloc\n");
    return 0;
  }
  copySize = GET_SIZE(HDRP(ptr));
  if (size < copySize) {
    copySize = size;
  }
  memcpy(newp, ptr, copySize);
  mm_free(ptr);
  return newp;
      
}


/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */

/* $begin mmplace */
static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    size_t split_size = csize - asize;
	
    size_t avg = (GETSIZE(NEXT_BLKP(bp)) + GETSIZE(PREV_BLKP(bp)))/2; 
    void* large;
    int split_side;
    void* split;
    void *blk;
    if (split_size >= 2*DSIZE) 
    {    
        
        if(GETSIZE(NEXT_BLKP(bp)) > GETSIZE(PREV_BLKP(bp)))
        {
            large = NEXT_BLKP(bp);
        }
        else
        {
            large = PREV_BLKP(bp);
        }
         
        if(asize > avg)
        {
            if(PREV_BLKP(bp) == large)
                split_side = 0;
            else 
                split_side = 1;           
        }
        else
        {
            if(PREV_BLKP(bp) == large)
                split_side = 1;
            else 
                split_side = 0;
        }
        
        if(split_side != 1)
        {
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(FTRP(bp), PACK(asize, 1));

            split = NEXT_BLKP(bp);

            PUT(HDRP(split), PACK(csize-asize, 0));
            PUT(FTRP(split), PACK(csize-asize, 0));

            root_node = insert(root_node,split);

            return bp;
        }
        else
        {
            PUT(HDRP(bp), PACK(split_size,0));
            PUT(FTRP(bp), PACK(split_size,0));
        
            blk = NEXT_BLKP(bp);

            PUT(HDRP(blk), PACK(asize, 1));
            PUT(FTRP(blk), PACK(asize, 1));

            root_node = insert(root_node,bp);

            return blk;
        }
    }
    else
    { 
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        return bp;
    }

}
/* $end mmplace */



/* 
 * mm_checkheap - Check the heap for consistency 
 */
void mm_checkheap(int verbose) 
{
    char *bp = heap_listp;

    if (verbose) {
        printf("Heap (%p):\n", heap_listp);
        printf("Root (%p):\n", root_node);
    }

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);

        checkblock(bp);
    }
     
    if (verbose)
        printblock(bp);

    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

/* The remaining routines are internal helper routines */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;
    
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((int)(bp = mem_sbrk(size)) == -1) 
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}
/* $end mmextendheap */




/*
 * coalesce - boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin mmfree */
static void *coalesce(void *bp) 
{
    size_t palloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t nalloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    size_t next_nodesize = GET_SIZE(FTRP(NEXT_BLKP(bp)));
    size_t prev_nodesize = GET_SIZE(HDRP(PREV_BLKP(bp)));

    //next and prev are allocated
    if(palloc && nalloc)
    {            
        return bp;
    }
    else if(palloc && !nalloc) //next block is not allocated 
    {      
        size = size + next_nodesize;
        root_node = delete(root_node, NEXT_BLKP(bp)); //coalesce next block
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
        return bp;
    }
    else if (!palloc && nalloc) //prev block not allocated 
    {      
        size = size + prev_nodesize;
        root_node = delete(root_node, PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return PREV_BLKP(bp);
    }
    else //prev and next are not allocated
    {                                     
        size = prev_nodesize + size  +  next_nodesize;
        root_node = delete(root_node, NEXT_BLKP(bp));
        root_node = delete(root_node, PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        return PREV_BLKP(bp);
    }
}

/* $end mmfree */

static void printblock(void *bp) 
{
    size_t hsize, halloc, fsize, falloc;

  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));  
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));  
    
  if (hsize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }

  printf("%p: header: [%d:%c] footer: [%d:%c]\n",
	 bp, 
	 (int) hsize, (halloc ? 'a' : 'f'), 
	 (int) fsize, (falloc ? 'a' : 'f')); 


}

static void checkblock(void *bp) 
{
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

/*
 * insert - Insert a free block into the Binary Tree and return bp
 */
void *insert(void *root, void* bp)
{
    //binary tree insert operation 

    if(root == NULL)
    {
        PUT_LEFT(bp, NULL);
        PUT_RIGHT(bp, NULL);
        return bp;
    }
    else if(GETSIZE(bp) <= GETSIZE(root)) //size < root traverse left
    {
        PUT_LEFT(root, insert(LEFT_CHLD(root),bp));
    }
    else if(GETSIZE(bp) >  GETSIZE(root)) //size > root traverse right
    {
        PUT_RIGHT(root, insert(RIGHT_CHLD(root),bp));
    }
    
    return root;
}

/*
 * delete - Remove a node from the Binary Tree
 */
void *delete(void *root, void *bp)
{
    //binary tree delete operation
    
    int child_count = mm_children(bp); 
    void *parent = parent_node(root, bp);

    if( child_count == 0)
    {
    
       if(parent != NULL)
       {
           if(LEFT_CHLD(parent) == bp) 
               PUT_LEFT(parent, NULL);
           else 
               PUT_RIGHT(parent, NULL);
           
           return root;
       } 
       else
       {
           return NULL;
       } 

    }
    else if(child_count == 1)
    {
        
       void *child=NULL;
    
   
       if(LEFT_CHLD(bp) != NULL)
           child = LEFT_CHLD(bp);
       else
           child = RIGHT_CHLD(bp);

       if(parent != NULL)
       {
           if(LEFT_CHLD(parent) == bp)
               PUT_LEFT(parent, child);
           else 
               PUT_RIGHT(parent, child); 
        
          return root;
       }
       else
       {
           return child;                               
       }

    }
    else
    {
       
       void *replacement = inorder_replace(LEFT_CHLD(bp));
       void *new_bp;
    
       new_bp = delete(LEFT_CHLD(bp), replacement);
    
       PUT_LEFT(replacement, new_bp);
       PUT_RIGHT(replacement, RIGHT_CHLD(bp));
    
       if(parent != NULL) 
       {
        if(LEFT_CHLD(parent) == bp)
            PUT_LEFT(parent, replacement);
        else
            PUT_RIGHT(parent, replacement);
        return root;
       }
       else
       {
        return replacement; 
       }

    }
} 


/*
 * parent_node - Retrieve the parent node of a child node
 */
void *parent_node(void *root, void *bp)
{
    if(bp == root)
        return NULL;

    if(GETSIZE(bp) <= GETSIZE(root))
    {
        if(LEFT_CHLD(root) == bp)
            return root;
        else
            return parent_node(LEFT_CHLD(root),bp);
    }
    else
    {
        if(RIGHT_CHLD(root) == bp)
            return root;
        else
            return parent_node(RIGHT_CHLD(root),bp);
    }
} 

/*
 * mm_children - Return the number of children under a given root node
 */
int mm_children(void *root)
{
    int child_num = 0;
    if(LEFT_CHLD(root) != NULL)
         child_num++;
    if(RIGHT_CHLD(root) != NULL)
         child_num++;
                            
    return child_num;
}         


/*
 * inorder_replace - Locate the replacement for the parents' two children
 */
void *inorder_replace(void *bp)
{
     if(RIGHT_CHLD(bp) == NULL)
        return bp;
     else
        return inorder_replace(RIGHT_CHLD(bp));
}
