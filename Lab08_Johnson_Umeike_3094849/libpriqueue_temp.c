l/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.
  
  Assumptions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two itements. If comparer(x, y) < 0, then the priority of x is higher than the priority of y and therefore should be placed earlier in the queue.
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->comparer = comparer;
  q->top = NULL;  //top always points to the item at the head of the queue
  q->tail = NULL; //top always points to the item at the tail of the queue
  q->size = 0;  
}


/**
  Insert the specified itement into this priority queue. You should use the queue's `comparer` function to determine where to place the item.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  
  node_t *new_item = (node_t *) malloc(sizeof(node_t));
  new_item->data = ptr;
  new_item->next = NULL;
  new_item->prev = NULL;

  node_t* curr_item = q->top;

  if(curr_item == NULL) { //empty queue
    q->top = new_item;
    new_item->prev = NULL;
    q->tail = new_item;
    new_item->next = NULL;
    q->size++;
    return 0;
  }
  
  int index = 0;
  while(curr_item != NULL) {
    if(q->comparer(new_item->data, curr_item->data) < 0) { //if the new item has a higher priority
      if(curr_item->prev == NULL) { // if the current item is at the top
        new_item->prev == NULL;
        new_item->next = curr_item;
        curr_item->prev = new_item;
        q->top = new_item;
        q->size++
        return index;
      }
      else {
        new_item->prev = curr_item->prev;
        curr_item->prev = new_item;
        new_item->next = curr_item;
        q->size++;
        return index;
      }
      curr_item = curr_item->next;
      index++;
    }
  }
  //if new item has the least priority from current list of items
  new_item->prev = q->tail;
  q->tail = new_item;
  new_item->next = NULL;
  q->size++;
	return index;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to itement at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  node_t* item = q->top;
  if(item != NULL) //queue is not empty
    return item->data;
	else
    return NULL;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  node_t* item = q->top;
  if(item != NULL){ // queue is not empty
    (item->next)->prev = NULL;
    q->top = item->next;
    q->size--;
    return item->data;
  }
	else
    return NULL;
}


/**
  Returns the itement at the specified position in this list, or NULL if
  the queue does not contain an index'th itement.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved itement
  @return the index'th itement in the queue
  @return NULL if the queue does not contain the index'th itement
 */
void *priqueue_at(priqueue_t *q, int index)
{
  node_t* item = q->top;
  int top_index = 0;
  if(index == top_index)
    return item->data;

  while(top_index < q->size) {
    
    top_index++;
    item = item->next;
    
    if(top_index == index)
      return item->data;
  }
	return NULL;
}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each itement of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of itement to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  node_t * item;
  item = q->top;
  int count =0;
  while(item != NULL){
    if(*(int *)(item->data) == *(int *)ptr){
      node_t * toDelete = item;
      if(item->prev == NULL && item->next == NULL){//Single item
        q->queue = NULL;
        
      }
      else if(item->prev == NULL){//head
        ((node_t *)(item->next))->prev = NULL;
        q->queue = ((node_t *)(item->next));
        
      }
      else if(item->next == NULL){//tail
        ((node_t *)(item->prev))->next = NULL;
        q->tail = (node_t *)(item->prev);
        
      }
      else{//neither
        ((node_t *)item->next)->prev = item->prev;
        ((node_t *)item->prev)->next = item->next;
      }
      item = item->next;
      q->size--;
      count++;
    }
    else{
      item = (node_t *)item->next;
    }
  }
  return count;
  // node_t* item = q->top;
  // int* data = item->data;
  // int top_index = 0;
  // int count = 0;
  
  // if(*data == *(int*)ptr) {
  //   count++;
    
  //   if(item->next != NULL){ //if we have more than one item in the list
  //     (item->next)->prev = NULL;
  //     item = item->next;
  //     q->top = item;
  //     q->size--;
  //   }
  //   else{
  //     q->tail = NULL;
  //     return count;
  //   }  
  // }
  
  // do {
  //   top_index++;
    
  //   if(*data == *(int*)ptr) {
  //     count++;
  //     if(item->prev == NULL)
  //       q->top = item->next;
  //     if(item->next != NULL){ 
  //       (item->next)->prev = item->prev;
  //       q->size--;
  //     }
      
  //     if(item->next == NULL){
  //       q->tail == NULL;
  //       return count;
  //     }
  //     else if(item->prev == NULL) {
        
  //     }
  //   }
  // } while(item->next != NULL)

	return 0;
}


/**
  Removes the specified index from the queue, moving later itements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of itement to be removed
  @return the itement removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	return 0;
}


/**
  Return the number of itements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of itements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return 0;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{

}
