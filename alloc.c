#define _DEFAULT_SOURCE

#include "alloc.h"
#include <unistd.h> // NULL, sbrk()

int HEAP_SIZE = 0;
int MAX_HEAP_SIZE = 0;
enum algs ALLOC_ALGORITHM = FIRST_FIT;
void *initial_break = NULL;

// Pointer to the head of the free list
struct header *free_list = NULL;

void *alloc(int block_size) {

  // allocopt() not called || invalid block size
  if (initial_break == NULL || block_size < 0) {
    return NULL;
  }

  // Iterate through free_list for free block
  uint64_t min_size =
      (uint64_t)(block_size + sizeof(struct header));
  struct header *cur_block = free_list;
  struct header *prev_block = NULL;
  struct header *prev_fit_block = NULL;
  struct header *fit_block = NULL;

  // Choose algorithm
  switch (ALLOC_ALGORITHM) {
  case FIRST_FIT:
    while (cur_block != NULL) {
      if (cur_block->size >= min_size) {
        // Found suitable block
        break;
      }
      prev_block = cur_block;
      cur_block = cur_block->next;
    }
    break;
  case BEST_FIT:
    while (cur_block != NULL) {
      if (cur_block->size >= min_size) {
        if (fit_block == NULL || cur_block->size < fit_block->size) {
          prev_fit_block = prev_block;
          fit_block = cur_block;
        }
      }
      prev_block = cur_block;
      cur_block = cur_block->next;
    }
    break;
  case WORST_FIT:
    while (cur_block != NULL) {
      if (cur_block->size >= min_size) {
        if (fit_block == NULL || cur_block->size > fit_block->size) {
          prev_fit_block = prev_block;
          fit_block = cur_block;
        }
      }
      prev_block = cur_block;
      cur_block = cur_block->next;
    }
    break;
  }

  if (fit_block != NULL) {
    prev_block = prev_fit_block;
    cur_block = fit_block;
  }

  if (cur_block == NULL) {
    // No suitable block found
    if (HEAP_SIZE + INCREMENT > MAX_HEAP_SIZE) {
      // New heap size > threshold
      return NULL;
    }

    // Allocate new block
    struct header *new_block = (struct header *)sbrk(INCREMENT);
    if (new_block == (struct header *)-1) {
      // sbrk() failed
      return NULL;
    }

    // Increase heap size counter
    HEAP_SIZE += INCREMENT;

    // Initialize block size
    new_block->size = (uint64_t)INCREMENT;

    // Insert new block at front of free list
    new_block->next = free_list;
    free_list = new_block;

    // Reassign cur block and prev block
    cur_block = new_block;
    prev_block = NULL;
  }

  // Set return pointer
  void *ret_ptr = (void *)((char *)cur_block + sizeof(struct header));

  // Set return size and save prev size
  uint64_t prev_size = cur_block->size;
  cur_block->size = (uint64_t)(block_size + sizeof(struct header));

  // Maintain free list
  if (prev_size > cur_block->size + sizeof(struct header)) {
    // Create new free block
    struct header *free_block =
        (struct header *)((char *)cur_block + cur_block->size);
    free_block->size = prev_size - cur_block->size;
    free_block->next = cur_block->next;
    // Insert free block into the free list
    if (prev_block != NULL) {
      // Insert where cur block was
      prev_block->next = free_block;
    } else {
      // No prev block: new front of list
      free_list = free_block;
    }
  } else {
    // Remove block from list
    if (prev_block != NULL) {
      prev_block->next = cur_block->next;
    } else {
      free_list = free_list->next;
    }
    cur_block->size = prev_size;
  }

  return ret_ptr;
}

void dealloc(void *ptr) {
  if (ptr == NULL) {
    // Invalid ptr
    return;
  }

  // Get header address from user ptr
  struct header *block = (struct header *)((char *)ptr - sizeof(struct header));

  // Add block to the free list
  block->next = free_list;
  free_list = block;

  // Merge contiguous free blocks
  struct header *cur_block = free_list;
  while (cur_block != NULL && cur_block->next != NULL) {
    struct header *next_block = cur_block->next;

    // Get expected end addr
    char *expected_end =
        (char *)cur_block + cur_block->size + sizeof(struct header);

    if (expected_end == (char *)next_block) {
      // Merge
      cur_block->size += next_block->size + sizeof(struct header);
      cur_block->next = next_block->next;
    } else {
      cur_block = cur_block->next;
    }
  }
}

void allocopt(enum algs alg, int heap_limit) {
  // Reset allocator
  if (initial_break != NULL) {
    brk(initial_break);
  } else {
    initial_break = sbrk(0);
  }

  // Set options
  HEAP_SIZE = 0;
  free_list = NULL;
  MAX_HEAP_SIZE = heap_limit;
  ALLOC_ALGORITHM = alg;
}

struct allocinfo allocinfo() {
  struct allocinfo info;
  info.free_size = 0;
  info.free_chunks = 0;
  info.largest_free_chunk_size = 0;
  info.smallest_free_chunk_size = MAX_HEAP_SIZE;

  struct header *cur_block = free_list;

  while (cur_block != NULL) {
    info.free_size += cur_block->size;
    info.free_chunks++;

    if (cur_block->size > info.largest_free_chunk_size) {
      info.largest_free_chunk_size = cur_block->size;
    }

    if (cur_block->size < info.smallest_free_chunk_size) {
      info.smallest_free_chunk_size = cur_block->size;
    }

    cur_block = cur_block->next;
  }

  if (info.free_size > 0) {
    info.free_size -= sizeof(struct header);
  }

  return info;
}
