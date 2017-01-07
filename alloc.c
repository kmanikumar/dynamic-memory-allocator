/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct metadata{
  struct metadata * next;
  struct metadata * prev;
  size_t size;
  int free;
} md;


md * head = NULL;
md * tail = NULL;
md * max = NULL;
md * max2 = NULL;

void coalesce(md * curr){
  md * neighbour= curr->next; //coalesce next neighbour
  if(neighbour && neighbour->free){
    curr->size = curr->size + neighbour->size + sizeof(md); //update size
    curr->next = neighbour->next;//update next ptr
    if(neighbour->next){
      neighbour->next->prev = curr;
    }
    if(neighbour == tail){ //update tail ptr
      tail = curr;
    }
  }
  md * neighbour2 = curr->prev; //coalesce prev neighbour
  if(neighbour2 && neighbour2->free){
    neighbour2->size = neighbour2->size + curr->size + sizeof(md);//update size
    neighbour2->next = curr->next;//update next ptr
    if(curr->next){
      curr->next->prev = neighbour2;
    }
    if(curr == tail){ //update tail ptr
      tail = neighbour2;
    }
  }
  if(!max || curr->size > max->size){ //updates maxes
    max2 = max;
    max = curr;
  }
}

void split(md * ret, int size){
  if(!ret){ //null ptr
    return;
  }
  else{

    md * newblock = (md *)((char *) ret + (size + sizeof(md)));
    newblock->free = 1;
    newblock->size = ret->size - (size + sizeof(md));
    ret->size = size;

    newblock->next = ret->next;
    if(ret->next){
      (ret->next)->prev = newblock;
    }
    newblock->prev = ret;
    ret->next = newblock;

    if(ret == tail){
      tail = newblock;
    }

    if(!max || (max->size < newblock->size)){
      max2 = max;
      max = newblock;
    }

  }
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size){
  // implement calloc!
  void * ret = malloc(num * size);
  if(ret){
    memset(ret, 0, num*size);
  }
  return ret;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size){
  // implement malloc!
  if(!size){
    return NULL;
  }
  md * curr = tail;
  md * ret = NULL;

  if(max && size <= max->size){
    while (curr){
      if((curr->free == 1 && curr->size >= size)){
        ret = curr;
        break;
      }
      curr = curr->prev;
    }
  }

  if(ret == max){
    max = max2;
  }

  /*if(ret != NULL){
    if(ret->size > (sizeof(md) + size) ){
      //split(ret, size);
    }
    if(head == ret){
      head = ret->next;
    }
    else{

    }
    return (void *)(ret + 1);
  }
  else{
    ret = sbrk(sizeof(md) + size);
    if( ret == (void *)(-1) ){ return NULL; } //sbrk() failed

    ret->size = size;

    return (void *)(ret + 1);
  }*/

  if(!ret){
    ret = sbrk(size + 3 * sizeof(md));
    if(ret == (void *)(-1)){return NULL;}
    ret->size = size + 2 * sizeof(md);
    ret->prev = tail;
    if(!head){
      head = ret;
    }
    if(tail)
      tail->next = ret;
    tail = ret;
    tail->next = NULL;
  }

  ret->free = 0;
  if(ret->size > (size + sizeof(md)) && (ret->size - size - sizeof(md)) > 128){ // 128B minimum remainder to make sure it blocks dont become too small and fragmented
    split(ret, size);
  }
  return (void*)(ret + 1);
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr){
  // implement free!
  if(!ptr){ return; } //null pointer

  md * curr = (md *) ptr;
  curr--;
  curr->free = 1;
  coalesce(curr);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size){
  // implement realloc!
  if(!ptr){ //null pointer
    return malloc(size);
  }

  if(!size){ //size ==0
    free(ptr);
    return NULL;
  }

  md * p = (md *) ptr;
  p--;

  if(p->size >= size && p->size <= (size + sizeof(md))){
    return ((void*)(p + 1));
  }

  void * new = malloc(size);
  if(!new){ //malloc failed
    return NULL;
  }

  if(p->size < size){
    memcpy(new, ptr, p->size);
  }
  else{
    memcpy(new, ptr, size);
  }
  free(ptr);
  return new;
}
