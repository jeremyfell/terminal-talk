// Manages the thread that prints output to the terminal
#ifndef _OUTPUT_H_
#define _OUTPUT_H_
#include "threadsafelist.h"

// Arguments for the output thread
typedef struct {
  List* pReceivedMessagesList;
} OutputThreadArguments;

// Initializes the output thread
void Output_init(OutputThreadArguments* pOutputArguments);

// Signals the output thread that there is a received message
void Output_signalMessageReceived(void);

// Shutdowns the output thread and performs necessary cleanup
void Output_shutdown(void);

#endif
