#include <stddef.h>
#include <assert.h>
#include "list.h"

// Statically allocated array of nodes for use in lists
static Node s_nodeArray[LIST_MAX_NUM_NODES];

// Statically allocated array of list heads
static List s_headArray[LIST_MAX_NUM_HEADS];

// Pointer to the next available node that can be used in a list
// NULL if there are no remaining available nodes
static Node* s_pNextAvailableNode;

// Pointer to the next available list head that can be used
// NULL if there are no remaining list heads
static List* s_pNextAvailableHead;

// Dummy nodes
static Node s_beforeListStartPlaceholder;
static Node s_beyondListEndPlaceholder;

// Constant node pointers to represent the current node of a list being
// either before the start of the list or beyond the end of the list
static Node* BEFORE_LIST_START = &s_beforeListStartPlaceholder;
static Node* BEYOND_LIST_END = &s_beyondListEndPlaceholder;

#define NOT_INITIALIZED 0

// Tracks whether the global variables have been initialized
static bool s_initializationIsDone = NOT_INITIALIZED;


// Frees the node, allowing it to be available for another list
// Note: does not free the item associated with the node
static void freeNode(Node* pNode) {
  assert(pNode != NULL);

  pNode->pItem = NULL;
  pNode->pNextNode = s_pNextAvailableNode;
  pNode->pPrevNode = NULL;
  s_pNextAvailableNode = pNode;

  return;
}

// Frees the list head, allowing it to be used in another list
// Note: does not free the nodes in the list
static void freeHead(List* pList) {
  assert(pList != NULL);

  pList->pNextHead = s_pNextAvailableHead;
  pList->size = 0;
  pList->pCurrentNode = BEFORE_LIST_START;
  pList->pHeadNode = NULL;
  pList->pTailNode = NULL;
  s_pNextAvailableHead = pList;

  return;
}

// Makes a new node with the provided item, and returns its reference on success
// Returns a NULL pointer on failure
static Node* createNode(void* pItem, Node* pPrevNode, Node* pNextNode) {
  if (s_pNextAvailableNode == NULL) {
    return NULL; // Failure, no more available nodes
  }

  // Create new node from the first available node
  Node* pNewNode = s_pNextAvailableNode;
  s_pNextAvailableNode = s_pNextAvailableNode->pNextNode;
  pNewNode->pItem = pItem;
  pNewNode->pPrevNode = pPrevNode;
  pNewNode->pNextNode = pNextNode;

  return pNewNode;
}

// Sets up the data structures needed to create lists
static void initialization() {
  // Set up dummy nodes
  s_beforeListStartPlaceholder.pNextNode = NULL;
  s_beforeListStartPlaceholder.pPrevNode = NULL;
  s_beforeListStartPlaceholder.pItem = NULL;
  s_beyondListEndPlaceholder.pNextNode = NULL;
  s_beyondListEndPlaceholder.pPrevNode = NULL;
  s_beyondListEndPlaceholder.pItem = NULL;

  // Set up up linked chain of available nodes and linked chain of available list heads
  s_pNextAvailableNode = &s_nodeArray[0];
  s_pNextAvailableHead = &s_headArray[0];

  for (int i = 0; i < LIST_MAX_NUM_NODES - 1; i++) {
    s_nodeArray[i].pNextNode = &s_nodeArray[i + 1];
  }
  s_nodeArray[LIST_MAX_NUM_NODES - 1].pNextNode = NULL;

  for (int i = 0; i < LIST_MAX_NUM_HEADS - 1; i++) {
    s_headArray[i].pNextHead = &s_headArray[i + 1];
  }
  s_headArray[LIST_MAX_NUM_HEADS - 1].pNextHead = NULL;

  // Set flag so initialization only happens once
  s_initializationIsDone = 1;
  return;
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List* List_create() {

  // Initialize data structures when List_create is called for the first time
  if (!s_initializationIsDone) {
    initialization();
  }

  if (s_pNextAvailableHead == NULL) {
    return NULL; // Failure, no more available list heads
  }

  // Create new list from the first available list head
  List* pNewList = s_pNextAvailableHead;
  s_pNextAvailableHead = s_pNextAvailableHead->pNextHead;
  pNewList->pNextHead = NULL;
  pNewList->size = 0;
  pNewList->pCurrentNode = BEFORE_LIST_START;
  pNewList->pHeadNode = NULL;
  pNewList->pTailNode = NULL;

  return pNewList;
}

// Returns the number of items in pList.
int List_count(List* pList) {
  assert(pList != NULL);

  return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
  assert(pList != NULL);

  // Handle empty list
  if (pList->size == 0) {
    pList->pCurrentNode = BEFORE_LIST_START;
    return NULL;
  }

  assert(pList->pHeadNode != NULL);
  pList->pCurrentNode = pList->pHeadNode;
  return pList->pHeadNode->pItem;
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
  assert(pList != NULL);

  // Handle empty list
  if (pList->size == 0) {
    pList->pCurrentNode = BEYOND_LIST_END;
    return NULL;
  }

  assert(pList->pTailNode != NULL);
  pList->pCurrentNode = pList->pTailNode;
  return pList->pTailNode->pItem;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
  assert(pList != NULL);

  // If list is empty, or the current node is beyond the end of list, or the current node is the last node
  if (pList->size == 0 || pList->pCurrentNode == BEYOND_LIST_END || pList->pCurrentNode == pList->pTailNode) {
    pList->pCurrentNode = BEYOND_LIST_END;
    return NULL;
  }

  // If current item is before the start of list, and list is nonempty
  if (pList->pCurrentNode == BEFORE_LIST_START) {
    assert(pList->pHeadNode != NULL);
    pList->pCurrentNode = pList->pHeadNode;
    return pList->pHeadNode->pItem;
  }

  // If current node is any non-last node
  assert(pList->pCurrentNode != NULL);
  assert(pList->pCurrentNode->pNextNode != NULL);
  pList->pCurrentNode = pList->pCurrentNode->pNextNode;
  return pList->pCurrentNode->pItem;
}

// Backs up pList's current item by one, and returns a pointer to the new current item.
// If this operation backs up the current item beyond the start of the pList, a NULL pointer
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
  assert(pList != NULL);

  // If list is empty, or the current node is before the list, or the current node is the first node
  if (pList->size == 0 || pList->pCurrentNode == BEFORE_LIST_START || pList->pCurrentNode == pList->pHeadNode) {
    pList->pCurrentNode = BEFORE_LIST_START;
    return NULL;
  }

  // If current item is beyond end of list, and list is nonempty
  if (pList->pCurrentNode == BEYOND_LIST_END) {
    assert(pList->pTailNode != NULL);
    pList->pCurrentNode = pList->pTailNode;
    return pList->pTailNode->pItem;
  }

  // If current node is any non-first node
  assert(pList->pCurrentNode != NULL);
  assert(pList->pCurrentNode->pPrevNode != NULL);
  pList->pCurrentNode = pList->pCurrentNode->pPrevNode;
  return pList->pCurrentNode->pItem;
}

// Returns a pointer to the current item in pList.
// Returns NULL if current is before the start of the pList, or after the end of the pList.
void* List_curr(List* pList) {
  assert(pList != NULL);

  // If current item is before start of list or beyond end of list
  if (pList->pCurrentNode == BEFORE_LIST_START || pList->pCurrentNode == BEYOND_LIST_END) {
    return NULL;
  }

  assert(pList->pCurrentNode != NULL);
  return pList->pCurrentNode->pItem;
}

// Adds the new item to pList directly after the current item, and makes item the current item.
// If the current pointer is before the start of the pList, the item is added at the start.
// If the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem) {
  assert(pList != NULL);

  // Current item is before start of list
  if (pList->pCurrentNode == BEFORE_LIST_START) {
    return List_prepend(pList, pItem);
  }

  // Current node is last node, or beyond end of list
  if (pList->pCurrentNode == pList->pTailNode || pList->pCurrentNode == BEYOND_LIST_END) {
    return List_append(pList, pItem);
  }

  // Current node is any non-last node
  assert(pList->size != 0);
  assert(pList->pCurrentNode != NULL);
  assert(pList->pCurrentNode->pNextNode != NULL);
  Node* pNewNode = createNode(pItem, pList->pCurrentNode, pList->pCurrentNode->pNextNode);

  if (pNewNode == NULL) {
    return -1; // Failure, node could not be created
  }

  pList->pCurrentNode->pNextNode->pPrevNode = pNewNode;
  pList->pCurrentNode->pNextNode = pNewNode;
  pList->pCurrentNode = pNewNode;
  pList->size++;
  return 0;
}

// Adds item to pList directly before the current item, and makes the new item the current one.
// If the current pointer is before the start of the pList, the item is added at the start.
// If the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) {
  assert(pList != NULL);

  // Current node is beyond end of list
  if (pList->pCurrentNode == BEYOND_LIST_END) {
    return List_append(pList, pItem);
  }

  // Current node is first node, or before start of list
  if (pList->pCurrentNode == pList->pHeadNode || pList->pCurrentNode == BEFORE_LIST_START) {
    return List_prepend(pList, pItem);
  }

  // Current node is any non-first node
  assert(pList->size != 0);
  assert(pList->pCurrentNode != NULL);
  assert(pList->pCurrentNode->pPrevNode != NULL);
  Node* pNewNode = createNode(pItem, pList->pCurrentNode->pPrevNode, pList->pCurrentNode);

  if (pNewNode == NULL) {
    return -1; // Failure, node could not be created
  }

  pList->pCurrentNode->pPrevNode->pNextNode = pNewNode;
  pList->pCurrentNode->pPrevNode = pNewNode;
  pList->pCurrentNode = pNewNode;
  pList->size++;
  return 0;
}

// Adds item to the end of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
  assert(pList != NULL);

  Node* pNewNode = createNode(pItem, pList->pTailNode, NULL);

  if (pNewNode == NULL) {
    return -1; // Failure, node could not be created
  }

  if (pList->size == 0) {
    pList->pHeadNode = pNewNode;
  } else {
    pList->pTailNode->pNextNode = pNewNode;
  }

  pList->pTailNode = pNewNode;
  pList->size++;
  pList->pCurrentNode = pNewNode;
  return 0;
}

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
  assert(pList != NULL);

  Node* pNewNode = createNode(pItem, NULL, pList->pHeadNode);

  if (pNewNode == NULL) {
    return -1; // Failure, node could not be created
  }

  if (pList->size == 0) {
    pList->pTailNode = pNewNode;
  } else {
    pList->pHeadNode->pPrevNode = pNewNode;
  }

  pList->pHeadNode = pNewNode;
  pList->size++;
  pList->pCurrentNode = pNewNode;
  return 0;
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
  assert(pList != NULL);

  // List is empty or current item is before start of list or beyond end of list
  if (pList->size == 0 || pList->pCurrentNode == BEFORE_LIST_START || pList->pCurrentNode == BEYOND_LIST_END) {
    return NULL;
  }

  assert(pList->pCurrentNode != NULL);
  void* pCurrentItem = pList->pCurrentNode->pItem;
  Node* pRemoveNode = pList->pCurrentNode;

  if (pList->size == 1) {
    // Handle list with only one item
    pList->pHeadNode = NULL;
    pList->pTailNode = NULL;
    pList->pCurrentNode = BEFORE_LIST_START;

  } else if (pList->pCurrentNode == pList->pHeadNode) {
    // Handle current node is the first node
    pList->pHeadNode = pList->pHeadNode->pNextNode;
    pList->pHeadNode->pPrevNode = NULL;
    pList->pCurrentNode = pList->pHeadNode;

  } else if (pList->pCurrentNode == pList->pTailNode) {
    // Handle current node is the last node
    pList->pTailNode = pList->pTailNode->pPrevNode;
    pList->pTailNode->pNextNode = NULL;
    pList->pCurrentNode = BEYOND_LIST_END;

  } else {
    // Handle current node is any non-first, non-last node
    pList->pCurrentNode->pNextNode->pPrevNode = pList->pCurrentNode->pPrevNode;
    pList->pCurrentNode->pPrevNode->pNextNode = pList->pCurrentNode->pNextNode;
    pList->pCurrentNode = pList->pCurrentNode->pNextNode;

  }

  pList->size--;
  freeNode(pRemoveNode);
  return pCurrentItem;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1.
// pList2 no longer exists after the operation; its head is available for future operations.
void List_concat(List* pList1, List* pList2) {
  assert(pList1 != NULL);
  assert(pList2 != NULL);
  assert(pList1 != pList2);

  // Concatenation is only necessary if second list is nonempty
  if (pList2->size > 0) {
    if (pList1->size > 0) {
      pList1->pTailNode->pNextNode = pList2->pHeadNode;
      pList2->pHeadNode->pPrevNode = pList1->pTailNode;
    } else {
      pList1->pHeadNode = pList2->pHeadNode;
    }

    pList1->pTailNode = pList2->pTailNode;
    pList1->size += pList2->size;
  }

  freeHead(pList2);
  return;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are
// available for future operations.
// UPDATED: Changed function pointer type, May 19
void List_free(List* pList, FREE_FN pItemFreeFn) {
  assert(pList != NULL);
  assert(pItemFreeFn != NULL);

  // Iterate through the list from the start,
  // freeing each node and its associated item
  Node* pCurrentNode = pList->pHeadNode;
  while (pCurrentNode != NULL) {
    Node* pNextNode = pCurrentNode->pNextNode;
    (*pItemFreeFn)(pCurrentNode->pItem);
    freeNode(pCurrentNode);
    pCurrentNode = pNextNode;
  }

  freeHead(pList);
  return;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
  assert(pList != NULL);

  // Handle empty list
  if (pList->size == 0) {
      return NULL;
  }

  assert(pList->pTailNode != NULL);
  Node* pLastNode = pList->pTailNode;
  void* pLastItem = pList->pTailNode->pItem;

  if (pList->size == 1) {
    // Handle list with only one item
    pList->pHeadNode = NULL;
    pList->pTailNode = NULL;
    pList->pCurrentNode = BEFORE_LIST_START;
    pList->size = 0;

  } else {
    // Handle list with more than one item
    assert(pList->pTailNode->pPrevNode != NULL);
    pList->pTailNode = pList->pTailNode->pPrevNode;
    pList->pTailNode->pNextNode = NULL;
    pList->pCurrentNode = pList->pTailNode;
    pList->size--;

  }

  freeNode(pLastNode);
  return pLastItem;
}

// Search pList, starting at the current item, until the end is reached or a match is found.
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match,
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator.
//
// If a match is found, the current pointer is left at the matched item and the pointer to
// that item is returned. If no match is found, the current pointer is left beyond the end of
// the list and a NULL pointer is returned.
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
  assert(pList != NULL);
  assert(pComparator != NULL);

  Node* pCurrentNode = pList->pCurrentNode;

  // If current item is beyond the end of the list, do not search
  if (pCurrentNode == BEYOND_LIST_END) {
    return NULL;
  }

  // If current item is before the start of the list, set it to the first item
  if (pCurrentNode == BEFORE_LIST_START) {
    pCurrentNode = pList->pHeadNode;
  }

  assert(pList->size == 0 || (pCurrentNode != NULL && pCurrentNode != BEFORE_LIST_START));
  // Iterate through the list starting from the current item,
  // comparing each item with a comparison function and argument
  while (pCurrentNode != NULL) {
    if ( (*pComparator)(pCurrentNode->pItem, pComparisonArg) ) {
      pList->pCurrentNode = pCurrentNode;
      return pCurrentNode->pItem;
    }

    pCurrentNode = pCurrentNode->pNextNode;
  }

  // No matching item was found
  pList->pCurrentNode = BEYOND_LIST_END;
  return NULL;
}
