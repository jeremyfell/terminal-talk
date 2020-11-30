// A thread-safe wrapper for the List ADT
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "threadsafelist.h"
#include "list.h"

// Mutex for safely accessing list functions
static pthread_mutex_t s_listMutex = PTHREAD_MUTEX_INITIALIZER;

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List* ThreadSafeList_create() {
  int status = 0;
  List* pNewList = NULL;

  status = pthread_mutex_lock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not lock list mutex\n", stdout);
    exit(1);
  }

  pNewList = List_create();

  status = pthread_mutex_unlock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not unlock list mutex\n", stdout);
    exit(1);
  }

  return pNewList;
}

// Returns the number of items in pList.
int ThreadSafeList_count(List* pList) {
  int status = 0;
  int count;

  status = pthread_mutex_lock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not lock list mutex\n", stdout);
    exit(1);
  }

  count = List_count(pList);

  status = pthread_mutex_unlock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not unlock list mutex\n", stdout);
    exit(1);
  }

  return count;
}

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int ThreadSafeList_prepend(List* pList, void* pItem) {
  int status = 0;
  int prependStatus = 0;

  status = pthread_mutex_lock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not lock list mutex\n", stdout);
    exit(1);
  }

  prependStatus = List_prepend(pList, pItem);

  status = pthread_mutex_unlock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not unlock list mutex\n", stdout);
    exit(1);
  }

  return prependStatus;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* ThreadSafeList_trim(List* pList) {
  int status = 0;
  void* pItem = NULL;

  status = pthread_mutex_lock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not lock list mutex\n", stdout);
    exit(1);
  }

  pItem = List_trim(pList);

  status = pthread_mutex_unlock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not unlock list mutex\n", stdout);
    exit(1);
  }

  return pItem;
}

void ThreadSafeList_free(List* pList, FREE_FN pItemFreeFn) {
  int status = 0;

  status = pthread_mutex_lock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not lock list mutex\n", stdout);
    exit(1);
  }

  List_free(pList, pItemFreeFn);

  status = pthread_mutex_unlock(&s_listMutex);

  if (status) {
    fputs("[Error]: could not unlock list mutex\n", stdout);
    exit(1);
  }

  return;
}

// Cleans up internal variables
void ThreadSafeList_cleanup() {
  int status = 0;

  status = pthread_mutex_destroy(&s_listMutex);

  if (status) {
    fputs("[Error]: could not destroy list mutex\n", stdout);
  }

  return;
}
