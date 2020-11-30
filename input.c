#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "input.h"
#include "control.h"
#include "sender.h"
#include "threadsafelist.h"

static pthread_t s_threadInput;
static bool s_threadHasExited = false;

// Free any remaining memory
static void cleanup(void* args) {
  char** inputMessageAddress = args;
  char* inputMessage = *inputMessageAddress;

  if (inputMessage != NULL) {
    free(inputMessage);
  }

  return;
}

// The thread to handle keyboard input
static void* inputThread(void* args) {
  int status = 0;
  InputThreadArguments* inputArguments = args;
  List* pSendingMessagesList = inputArguments->pSendingMessagesList;

  bool isFirstSegment = true;
  char* input = NULL;
  char* inputMessage = NULL;
  char** inputMessageAddress = &inputMessage;

  pthread_cleanup_push(cleanup, inputMessageAddress);

  while (1) {
    inputMessage = malloc(MESSAGE_MAX_SIZE);

    if (inputMessage == NULL) {
      fputs("[Error]: could not allocate memory for input message\n", stdout);
      exit(1);
    }

    // Necessary to detect if received message is a new line or part of an existing line
    memset(inputMessage, 0, MESSAGE_MAX_SIZE);

    // Get keyboard input from the user
    input = fgets(inputMessage, MESSAGE_MAX_SIZE, stdin);

    // If EOF has been reached from a piped file without a !<enter>,
    // send the exit command anyways
    if (input == NULL) {
      strcpy(inputMessage, TERMINATE);
    } else {
      input = NULL;
    }

    // Detect if the program should be terminated, and if the current input is the
    // start of a new line (the first segment), or continues an existing line
    if (isFirstSegment && strcmp(inputMessage, TERMINATE) == 0) {
      s_threadHasExited = true;
    } else if (inputMessage[MESSAGE_MAX_SIZE - 2] != 0 && inputMessage[MESSAGE_MAX_SIZE - 2] != '\n') {
      isFirstSegment = false;
    } else {
      isFirstSegment = true;
    }

    // Add input to the end of the sending messages queue
    status = ThreadSafeList_prepend(pSendingMessagesList, inputMessage);

    if (status == -1) {
      fputs("[Error]: could not add the message to sending messages list\n", stdout);
      free(inputMessage);
      s_threadHasExited = false;
    }
    inputMessage = NULL;

    // Signal the sender thread that there is a message to send
    Sender_signalMessageToSend();

    if (s_threadHasExited) {
      fputs("[You have sent the exit command]\n", stdout);
      fflush(stdout);
      break;
    }
  }

  pthread_cleanup_pop(1);

  s_threadHasExited = true;

  // Signal the main thread that the program should terminate
  Control_signalTermination();

  return NULL;
}

// Initializes the input thread
void Input_init(InputThreadArguments* pInputArguments) {
  int status = 0;

  status = pthread_create(&s_threadInput, NULL, inputThread, (void*) pInputArguments);

  if (status) {
    fputs("[Error]: could not create input thread\n", stdout);
    exit(1);
  }

  return;
}

// Shutdowns the input thread and performs necessary cleanup
void Input_shutdown() {
  int status = 0;

  if (!s_threadHasExited) {
    status = pthread_cancel(s_threadInput);
  }

  if (status) {
    fputs("[Error]: could not cancel input thread\n", stdout);
  }

  status = pthread_join(s_threadInput, NULL);

  if (status) {
    fputs("[Error]: could not join with input thread\n", stdout);
  }

  return;
}
