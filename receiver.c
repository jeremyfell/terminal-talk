#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include "receiver.h"
#include "output.h"
#include "control.h"
#include "threadsafelist.h"

static pthread_t s_threadReceiver;

// Free any remaining memory
static void cleanup(void* args) {
  char** receivedMessageAddress = args;
  char* receivedMessage = *receivedMessageAddress;

  if (receivedMessage != NULL) {
    free(receivedMessage);
  }

  return;
}

// The thread to receive UDP messages
void* receiverThread(void* args) {
  int status = 0;
  ReceiverThreadArguments* receiverArguments = args;
  List* pReceivedMessagesList = receiverArguments->pReceivedMessagesList;
  int socketDescriptor = receiverArguments->socketDescriptor;

  char* receivedMessage = NULL;
  char** receivedMessageAddress = &receivedMessage;

  pthread_cleanup_push(cleanup, receivedMessageAddress);

  while (1) {
    struct sockaddr_in remoteSocket;
		unsigned int remoteLength = sizeof(remoteSocket);
		receivedMessage = malloc(MESSAGE_MAX_SIZE);

    if (receivedMessage == NULL) {
      fputs("[Error]: could not allocate memory for received message\n", stdout);
      exit(1);
    }

    // Necessary for output thread to detect if received message is a new
    // message or part of an existing message
    memset(receivedMessage, 0, MESSAGE_MAX_SIZE);

    // Get UDP message from the remote user
		int receivedLength = recvfrom(
      socketDescriptor, receivedMessage, MESSAGE_MAX_SIZE,
      0, (struct sockaddr *) &remoteSocket, &remoteLength
    );

    if (receivedLength == -1) {
      fputs("[Error]: could not receive message\n", stdout);
      exit(1);
    }

		// Make the message null terminated
    // Technically the sender does this, but just in case a corrupted packet is received
		int terminateIndex = (receivedLength < MESSAGE_MAX_SIZE) ? receivedLength : MESSAGE_MAX_SIZE - 1;
		receivedMessage[terminateIndex] = 0;

    // Add the message to the end of the received messages queue
    status = ThreadSafeList_prepend(pReceivedMessagesList, receivedMessage);

    if (status == -1) {
      fputs("[Error]: could not add message to received messages list\n", stdout);
      free(receivedMessage);
    }
    receivedMessage = NULL;

    // Signal the output thread that there is a received message
    Output_signalMessageReceived();
  }

  pthread_cleanup_pop(1);

  return NULL;
}

// Initializes the receiver thread
void Receiver_init(ReceiverThreadArguments* pReceiverArguments) {
  int status = 0;

  status = pthread_create(&s_threadReceiver, NULL, receiverThread, pReceiverArguments);

  if (status) {
    fputs("[Error]: could not create receiver thread\n", stdout);
    exit(1);
  }

  return;
}

// Shutdowns the receiver thread and performs necessary cleanup
void Receiver_shutdown() {
  int status = 0;

  status = pthread_cancel(s_threadReceiver);

  if (status) {
    fputs("[Error]: could not cancel receiver thread\n", stdout);
  }

  status = pthread_join(s_threadReceiver, NULL);

  if (status) {
    fputs("[Error]: could not join with receiver thread\n", stdout);
  }

  return;
}
