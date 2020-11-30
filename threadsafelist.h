// A thread-safe wrapper for the List ADT
#ifndef _THREADSAFELIST_H_
#define _THREADSAFELIST_H_
#include "list.h"

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List* ThreadSafeList_create();

// Returns the number of items in pList.
int ThreadSafeList_count(List* pList);

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int ThreadSafeList_prepend(List* pList, void* pItem);

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* ThreadSafeList_trim(List* pList);

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are
// available for future operations.
void ThreadSafeList_free(List* pList, FREE_FN pItemFreeFn);

// Cleans up internal variables
void ThreadSafeList_cleanup();

#endif
