#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "control.h"

static pthread_cond_t s_terminateCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_terminateMutex = PTHREAD_MUTEX_INITIALIZER;

// Wait for the program to be terminated by the local or remote user
void Control_waitForTermination() {
  int status = 0;

  status = pthread_mutex_lock(&s_terminateMutex);

  if (status) {
    fputs("[Error]: could not lock terminate mutex\n", stdout);
    exit(1);
  }

  status = pthread_cond_wait(&s_terminateCondition, &s_terminateMutex);

  if (status) {
    fputs("[Error]: could not wait on terminate condition variable\n", stdout);
    exit(1);
  }

  pthread_mutex_unlock(&s_terminateMutex);

  if (status) {
    fputs("[Error]: could not unlock terminate mutex\n", stdout);
    exit(1);
  }

  return;
}

// Signal that the program should be terminated
void Control_signalTermination() {
  int status = 0;

  status = pthread_mutex_lock(&s_terminateMutex);

  if (status) {
    fputs("[Error]: could not lock terminate mutex\n", stdout);
    exit(1);
  }

  status = pthread_cond_signal(&s_terminateCondition);

  if (status) {
    fputs("[Error]: could not signal terminate condition variable\n", stdout);
    exit(1);
  }

  status = pthread_mutex_unlock(&s_terminateMutex);

  if (status) {
    fputs("[Error]: could not unlock terminate mutex\n", stdout);
    exit(1);
  }

  return;
}

// Cleans up internal variables
void Control_cleanup() {
  int status = 0;

  status = pthread_cond_destroy(&s_terminateCondition);

  if (status) {
    fputs("[Error]: could not destroy terminate condition variable\n", stdout);
  }

  status = pthread_mutex_destroy(&s_terminateMutex);

  if (status) {
    fputs("[Error]: could not destroy terminate mutex\n", stdout);
  }

  return;
}
