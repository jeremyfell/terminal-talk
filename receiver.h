// Manages the thread that receives UDP messages
#ifndef _RECEIVER_H_
#define _RECEIVER_H_
#include "threadsafelist.h"
#include "control.h"

// Arguments for the receiver thread
typedef struct {
  List* pReceivedMessagesList;
  int socketDescriptor;
} ReceiverThreadArguments;

// Initializes the receiver thread
void Receiver_init(ReceiverThreadArguments* pReceiverArguments);

// Shutdowns the receiver thread and performs necessary cleanup
void Receiver_shutdown(void);

#endif
