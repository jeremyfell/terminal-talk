// Manages the thread that sends UDP messages
#ifndef _SENDER_H_
#define _SENDER_H_
#include "threadsafelist.h"
#include "control.h"

// Arguments for the sender thread
typedef struct {
  List* pSendingMessagesList;
  int socketDescriptor;
  int remotePort;
  char remoteHostName[HOSTNAME_MAX_SIZE];
} SenderThreadArguments;

// Initializes the sender thread
void Sender_init(SenderThreadArguments* pSenderArguments);

// Signals the sender thread that there is a message to send
void Sender_signalMessageToSend(void);

// Shutdowns the sender thread and performs necessary cleanup
void Sender_shutdown(void);

#endif
