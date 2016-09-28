#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);
#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */

#define RIGHT_CHLD(bp) ((void *)* (int *)(bp+WSIZE))
#define LEFT_CHLD(bp) ((void *)* (int *)(bp))
#define PUT_LEFT(bp, bq) (*(int *)(bp)) = (int)(bq)
#define PUT_RIGHT(bp, bq) (*(int *)(bp+WSIZE)) = (int)(bq)

static inline int ADJUSTSIZE(size_t size){
   return (((size) + DSIZE + 7) / DSIZE ) * DSIZE;	
}


static inline int GETSIZE(void *bp){
   return (*(int*) (bp-WSIZE)) & ~7;	
}

/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    const char *teamname; /* ID1+ID2 or ID1 */
    const char *name1;    /* full name of first member */
    const char *id1;      /* login ID of first member */
    const char *name2;    /* full name of second member (if any) */
    const char *id2;      /* login ID of second member */
} team_t;

extern team_t team;

