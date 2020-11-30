#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "sender.h"
#include "threadsafelist.h"

static pthread_t s_threadSender;
static pthread_cond_t s_messageToSendCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_messageToSendMutex = PTHREAD_MUTEX_INITIALIZER;

// Free any remaining memory and ensure mutex is unlocked
static void cleanup(void* args) {
  char** sendingMessageAddress = args;
  char* sendingMessage = *sendingMessageAddress;

  if (sendingMessage != NULL) {
    free(sendingMessage);
  }

  pthread_mutex_trylock(&s_messageToSendMutex);
  pthread_mutex_unlock(&s_messageToSendMutex);

  return;
}

// The thread to send UDP messages
void* senderThread(void* args) {
  int status = 0;
  SenderThreadArguments* senderArguments = args;
  List* pSendingMessagesList = senderArguments->pSendingMessagesList;
  int socketDescriptor = senderArguments->socketDescriptor;
  int remotePort = senderArguments->remotePort;
  char* remoteHostName = senderArguments->remoteHostName;

  char* sendingMessage = NULL;
  char** sendingMessageAddress = &sendingMessage;

  pthread_cleanup_push(cleanup, sendingMessageAddress);

  struct addrinfo* addressResults = NULL;
  struct sockaddr_in* result = NULL;
  struct addrinfo hints;
  struct sockaddr_in remoteAddress;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  // Get address of remote user
  status = getaddrinfo(remoteHostName, NULL, &hints, &addressResults);

  if (status) {
    fputs("[Error]: could not get address info of remote host name\n", stdout);
    exit(1);
  }

  result = (struct sockaddr_in*) addressResults->ai_addr;

  remoteAddress.sin_family = AF_INET;
  remoteAddress.sin_port = htons(remotePort);
  remoteAddress.sin_addr = ((struct in_addr) result->sin_addr);

  fputs("[Sending to remote user at ", stdout);
  fputs(inet_ntoa(remoteAddress.sin_addr), stdout);
  fputs("]\n", stdout);
  fflush(stdout);

  freeaddrinfo(addressResults);
  addressResults = NULL;
  result = NULL;

  while (1) {
    // If there are no messages to send, wait until one arrives
    if (ThreadSafeList_count(pSendingMessagesList) == 0) {
      status = pthread_mutex_lock(&s_messageToSendMutex);

      if (status) {
        fputs("[Error]: could not lock message to send mutex\n", stdout);
        exit(1);
      }

      status = pthread_cond_wait(&s_messageToSendCondition, &s_messageToSendMutex);

      if (status) {
        fputs("[Error]: could not wait on message to send condition variable\n", stdout);
        exit(1);
      }

      status = pthread_mutex_unlock(&s_messageToSendMutex);

      if (status) {
        fputs("[Error]: could not unlock message to send mutex\n", stdout);
        exit(1);
      }
    }

    // Get message from messages to send queue
    sendingMessage = ThreadSafeList_trim(pSendingMessagesList);

    if (sendingMessage == NULL) {
      fputs("[Error]: sending message was null\n", stdout);
      exit(1);
    }

    // Send UDP message to the remote user
		status = sendto(
      socketDescriptor, sendingMessage, strlen(sendingMessage),
			0, (struct sockaddr *) &remoteAddress, sizeof(remoteAddress)
    );

    if (status == -1) {
      fputs("[Error]: could not send message\n", stdout);
      exit(1);
    }

    free(sendingMessage);
    sendingMessage = NULL;
  }

  pthread_cleanup_pop(1);

  return NULL;
}

// Signals the sender thread that there is a message to send
void Sender_signalMessageToSend() {
  int status = 0;

  status = pthread_mutex_lock(&s_messageToSendMutex);

  if (status) {
    fputs("[Error]: could not lock message to send mutex\n", stdout);
    exit(1);
  }

  status = pthread_cond_signal(&s_messageToSendCondition);

  if (status) {
    fputs("[Error]: could not signal message to send condition variable\n", stdout);
    exit(1);
  }

  status = pthread_mutex_unlock(&s_messageToSendMutex);

  if (status) {
    fputs("[Error]: could not unlock message to send mutex\n", stdout);
    exit(1);
  }

  return;
}

// Initializes the sender threads
void Sender_init(SenderThreadArguments* pSenderArguments) {
  int status = 0;

  status = pthread_create(&s_threadSender, NULL, senderThread, pSenderArguments);

  if (status) {
    fputs("[Error]: could not create sender thread\n", stdout);
    exit(1);
  }

  return;
}

// Shutdowns the sender thread and performs necessary cleanup
void Sender_shutdown() {
  int status = 0;

  status = pthread_cancel(s_threadSender);

  if (status) {
    fputs("[Error]: could not cancel sender thread\n", stdout);
  }

  status = pthread_join(s_threadSender, NULL);

  if (status) {
    fputs("[Error]: could not join with sender thread\n", stdout);
  }

  status = pthread_cond_destroy(&s_messageToSendCondition);

  if (status) {
    fputs("[Error]: could not destroy message to send condition variable\n", stdout);
  }

  status = pthread_mutex_destroy(&s_messageToSendMutex);

  if (status) {
    fputs("[Error]: could not destroy message to send mutex\n", stdout);
  }

  return;
}
