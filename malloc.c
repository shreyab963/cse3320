//Shreya Bhatta 
//Student ID: 1001736276
//to execute first fit: env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
//to execute next fit: env LD_PRELOAD=lib/libmalloc-nf.so tests/ffnf
//to execute best fit: env LD_PRELOAD=lib/libmalloc-bf.so tests/bfwf
//to execute worst fit: env LD_PRELOAD=lib/libmalloc-wf.so tests/bfwf

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>  

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)




static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   struct _block* min= NULL;
   size_t minSize = INT_MAX;
   size_t currSize = 0;
   while(curr)
   {
      *last = curr;
      currSize = (int) (curr->size - size);
      if((curr ->free) && ((curr->size >=size) && (minSize >currSize)))
      {
         min = curr;
         minSize = currSize;
      }
      curr = curr->next;
   }
   curr = min;
#endif

#if defined WORST && WORST == 0
   /* Worst fit */
   struct _block *max = NULL;
   size_t maxSize = INT_MIN;
   while(curr)
   {
      if((curr->size >= size) && (curr->free))
      {
         if(curr->size < maxSize)
         {
            maxSize = curr->size;
            max = curr;
         }
      }  
      curr = curr->next;
   }
   curr = max;
#endif

#if defined NEXT && NEXT ==0
   /* next fit */
   struct _block *last_reuse ;
   while (last_reuse && !(last_reuse->size >= size && last_reuse->free)) 
   {
      *last = last_reuse;
       last_reuse = last_reuse->next;
   }
   if( !last_reuse) last_reuse =curr;
   while (last_reuse && !(last_reuse->size >= size && last_reuse->free)) 
   {
      *last = last_reuse;
       last_reuse = last_reuse->next;
   }
   curr = last_reuse;

#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
   num_mallocs = num_mallocs +1;
   num_requested = num_requested+size;
   
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: Split free _block if possible */

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      num_grows++;
      max_heap = max_heap + size;
   }
   else
   {
      num_reuses = num_reuses+1;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;
   

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

/*
 * \brief calloc
 *allocate memory for array of nmem elements of size bytes
 * set memory to zero
 * if nmemb or size is zero return null
 * if not return pointer to allocated memory
 */
void *calloc(size_t nmemb, size_t size)
{
  if(nmemb == 0 || size == 0)
  {
     return NULL;
  }
  struct _block* curr = malloc(nmemb*size);//allocate memory for array of
                                           //nmem elements of size bytes
  if(curr)
  {
  memset(curr,0,nmemb*size ); //if curr is not NULL, set memory to zero
  }
  return curr;
}

/*
 * \brief realloc
 *
 * change size of memory block pointed to by ptr to size bytes.
 * if the new size is larger than the old one, the added mem won't be initialized
 *if ptr is NULL, call malloc, if ptr is not NULL, free
 * return the new malloc value
 */
void *realloc(void* ptr, size_t size) 
{
   //size_t new_size;
   struct _block* curr = BLOCK_HEADER(ptr);
   if(ptr == NULL)
   {
    // new_size = (unsigned long)malloc(size);
     //return *new_size;
     return malloc(size);
   }
   else
   {
     struct _block* new;
     new= malloc(size);
     memcpy(new, ptr, curr->size);
     free(ptr);

     return new;
   } 
}
/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }
    num_frees = num_frees +1;
    
   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;

   num_blocks = num_blocks +1;

   /* TODO: Coalesce free _blocks if needed */
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
