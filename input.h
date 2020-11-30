// Manages the thread that handles keyboard input
#ifndef _INPUT_H_
#define _INPUT_H_
#include "threadsafelist.h"

// Arguments for the input thread
typedef struct {
  List* pSendingMessagesList;
} InputThreadArguments;

// Initializes the input thread
void Input_init(InputThreadArguments* pInputArguments);

// Shutdowns the input thread and performs necessary cleanup
void Input_shutdown(void);

#endif
