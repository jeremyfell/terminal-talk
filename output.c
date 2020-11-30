#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "output.h"
#include "control.h"
#include "threadsafelist.h"

static pthread_t s_threadOutput;
static bool s_threadHasExited = false;
static pthread_cond_t s_messageReceivedCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_messageReceivedMutex = PTHREAD_MUTEX_INITIALIZER;

// Free any remaining memory and ensure mutex is unlocked
static void cleanup(void* args) {
  char** receivedMessageAddress = args;
  char* receivedMessage = *receivedMessageAddress;

  if (receivedMessage != NULL) {
    free(receivedMessage);
  }

  pthread_mutex_trylock(&s_messageReceivedMutex);
  pthread_mutex_unlock(&s_messageReceivedMutex);

  return;
}

// The thread to print output to the terminal
static void* outputThread(void* args) {
  int status = 0;
  OutputThreadArguments* outputArguments = args;
  List* pReceivedMessagesList = outputArguments->pReceivedMessagesList;

  bool isFirstSegment = true;
  char* receivedMessage = NULL;
  char** receivedMessageAddress = &receivedMessage;

  pthread_cleanup_push(cleanup, receivedMessageAddress);

  while (1) {
    // If there are no received messages, wait until one arrives
    if (ThreadSafeList_count(pReceivedMessagesList) == 0) {
      status = pthread_mutex_lock(&s_messageReceivedMutex);

      if (status) {
        fputs("[Error]: could not lock message received mutex\n", stdout);
        exit(1);
      }

      status = pthread_cond_wait(&s_messageReceivedCondition, &s_messageReceivedMutex);

      if (status) {
        fputs("[Error]: could not wait on message received condition variable\n", stdout);
        exit(1);
      }

      status = pthread_mutex_unlock(&s_messageReceivedMutex);

      if (status) {
        fputs("[Error]: could not unlock message received mutex\n", stdout);
        exit(1);
      }
    }

    // Get message from received messages queue
    receivedMessage = ThreadSafeList_trim(pReceivedMessagesList);

    if (receivedMessage == NULL) {
      fputs("[Error]: received message was null\n", stdout);
      exit(1);
    }

    // Detect if the received message is the last part of an existing line
    bool isLastSegment = (receivedMessage[MESSAGE_MAX_SIZE - 2] == '\0' || receivedMessage[MESSAGE_MAX_SIZE - 2] == '\n');

    // Prints the received message to the terminal
    // Also detects if the program should be terminated
    if (isFirstSegment) {
      fputs("[Remote]: ", stdout);
      fputs(receivedMessage, stdout);

      if (strcmp(receivedMessage, TERMINATE) == 0) {
        fputs("[The remote user has sent the exit command]\n", stdout);
        s_threadHasExited = true;
      } else if (!isLastSegment) {
        isFirstSegment = false;
      }
    } else {
      fputs(receivedMessage, stdout);

      if (isLastSegment) {
        isFirstSegment = true;
      }
    }
    fflush(stdout);

    free(receivedMessage);
    receivedMessage = NULL;

    if (s_threadHasExited) {
      break;
    }
  }

  pthread_cleanup_pop(1);

  s_threadHasExited = true;

  // Signal the main thread that the program should terminate
  Control_signalTermination();

  return NULL;
}

// Signals the output thread that there is a received message
void Output_signalMessageReceived() {
  int status = 0;

  status = pthread_mutex_lock(&s_messageReceivedMutex);

  if (status) {
    fputs("[Error]: could not lock message received mutex\n", stdout);
    exit(1);
  }

  status = pthread_cond_signal(&s_messageReceivedCondition);

  if (status) {
    fputs("[Error]: could not signal message received condition variable\n", stdout);
    exit(1);
  }

  status = pthread_mutex_unlock(&s_messageReceivedMutex);

  if (status) {
    fputs("[Error]: could not unlock message received mutex\n", stdout);
    exit(1);
  }

  return;
}

// Initializes the output thread
void Output_init(OutputThreadArguments* pOutputArguments) {
  int status = 0;

  status = pthread_create(&s_threadOutput, NULL, outputThread, pOutputArguments);

  if (status) {
    fputs("[Error]: could not create output thread\n", stdout);
    exit(1);
  }

  return;
}

// Shutdowns the output thread and performs necessary cleanup
void Output_shutdown() {
  int status = 0;

  if (!s_threadHasExited) {
    status = pthread_cancel(s_threadOutput);
  }

  if (status) {
    fputs("[Error]: could not cancel output thread\n", stdout);
  }

  status = pthread_join(s_threadOutput, NULL);

  if (status) {
    fputs("[Error]: could not join with output thread\n", stdout);
  }

  status = pthread_cond_destroy(&s_messageReceivedCondition);

  if (status) {
    fputs("[Error]: could not destroy message received condition variable\n", stdout);
  }

  status = pthread_mutex_destroy(&s_messageReceivedMutex);

  if (status) {
    fputs("[Error]: could not destroy message received mutex\n", stdout);
  }

  return;
}
