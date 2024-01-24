# memory-allocator-C

Memory allocator in C. Mimics functions ```malloc()```, ```free()```, ```mallopt()```, ```mallinfo()```.<br />
Has options for First Fit, Best Fit, and Worst Fit algorithms.<br />
Intended to be included with ```#include <alloc.h>```.<br />
<br />
From alloc.h:<br />
```
struct allocinfo {
  int free_size;
  int free_chunks;
  int largest_free_chunk_size;
  int smallest_free_chunk_size;
};

/*
 * alloc() allocates memory from the heap. The first argument indicates the
 * size. It returns the pointer to the newly-allocated memory. It returns NULL
 * if there is not enough space.
 */
void *alloc(int);

/*
 * dealloc() frees the memory pointed to by the first argument.
 */
void dealloc(void *);

/*
 * allocopt() sets the options for the memory allocator.
 *
 * The first argument sets the algorithm. The second argument sets the size
 * limit.
 */
void allocopt(enum algs, int);

/*
 * allocinfo() returns the current statistics.
 */
struct allocinfo allocinfo(void);
