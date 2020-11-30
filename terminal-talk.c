#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include "control.h"
#include "threadsafelist.h"
#include "input.h"
#include "output.h"
#include "sender.h"
#include "receiver.h"

static InputThreadArguments s_inputArguments;
static OutputThreadArguments s_outputArguments;
static SenderThreadArguments s_senderArguments;
static ReceiverThreadArguments s_receiverArguments;

// Free a message stored in a list
static void freeMessage(void* pItem) {
  free(pItem);
  return;
}

// Sleep for the specified milliseconds
static void sleepMilliseconds(int msec) {
  struct timespec sleep_time;
  int usec = msec * 1000;
	sleep_time.tv_sec = (usec / 1000000);
	sleep_time.tv_nsec = (usec % 1000000) * 1000;
  nanosleep(&sleep_time, NULL);
  return;
}

// Creates the socket using the local IP address and port
static int bindSocket(int localPort) {
  int status = 0;
  struct sockaddr_in localAddress;
  memset(&localAddress, 0, sizeof(localAddress));
  localAddress.sin_family = AF_INET;
  localAddress.sin_port = htons(localPort);
  localAddress.sin_addr.s_addr = INADDR_ANY;

	// Create the socket for UDP
	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

  if (socketDescriptor == -1) {
    fputs("[Error]: could not create socket descriptor\n", stdout);
    exit(1);
  }

  // Bind the socket to the local port
	status = bind(socketDescriptor, (struct sockaddr*) &localAddress, sizeof(localAddress));

  if (status == -1) {
    fputs("[Error]: could not bind socket, already in use\n", stdout);
    exit(1);
  }

  return socketDescriptor;
}

// Main program
int main(int argc, char *argv[]) {
  int status = 0;

  // Check that enough arguments have been provided
  if (argc != 4) {
    fputs("[Error]: terminal-talk requires 3 arguments\n", stdout);
    exit(1);
  }

  // Create lists for sending/receiving messages
  List* pSendingMessagesList = ThreadSafeList_create();
  List* pReceivedMessagesList = ThreadSafeList_create();

  if (pSendingMessagesList == NULL || pReceivedMessagesList == NULL) {
    fputs("[Error]: could not create lists\n", stdout);
    exit(1);
  }

  // Get and validate local and remote port numbers
  int localPort = atoi(argv[1]);
  int remotePort = atoi(argv[3]);

  if (localPort < 1024 || localPort > 65535) {
    fputs("[Error]: local port number is not in the range [1024, 65535]\n", stdout);
    exit(1);
  }

  if (remotePort < 1024 || remotePort > 65535) {
    fputs("[Error]: remote port number is not in the range [1024, 65535]\n", stdout);
    exit(1);
  }

  // Create socket and bind it
  int socketDescriptor = bindSocket(localPort);

  // Fill argument structs for each thread
  s_inputArguments.pSendingMessagesList = pSendingMessagesList;
  s_outputArguments.pReceivedMessagesList = pReceivedMessagesList;
  s_senderArguments.pSendingMessagesList = pSendingMessagesList;
  s_senderArguments.socketDescriptor = socketDescriptor;
  s_senderArguments.remotePort = remotePort;
  strncpy(s_senderArguments.remoteHostName, argv[2], HOSTNAME_MAX_SIZE);
  s_senderArguments.remoteHostName[HOSTNAME_MAX_SIZE - 1] = '\0';
  s_receiverArguments.pReceivedMessagesList = pReceivedMessagesList;
  s_receiverArguments.socketDescriptor = socketDescriptor;

  // Create each thread
  Sender_init(&s_senderArguments);
  Receiver_init(&s_receiverArguments);
  Output_init(&s_outputArguments);
  Input_init(&s_inputArguments);

  // Block the main thread, and wait until the input or output threads signal termination
  Control_waitForTermination();

  // Sleep one second to allow the last messages in each list to be processed
  sleepMilliseconds(1000);

  // Shutdown each thread and join with it
  Sender_shutdown();
  Receiver_shutdown();
  Output_shutdown();
  Input_shutdown();

  // Close the socket
  status = close(socketDescriptor);

  if (status) {
    fputs("[Error]: could not close socket\n", stdout);
  }

  // Free dynamic memory for lists
  ThreadSafeList_free(pReceivedMessagesList, freeMessage);
  pReceivedMessagesList = NULL;
  ThreadSafeList_free(pSendingMessagesList, freeMessage);
  pSendingMessagesList = NULL;

  // Additional cleanup
  ThreadSafeList_cleanup();
  Control_cleanup();

  fputs("[Program terminated successfully]\n", stdout);
  fflush(stdout);

  return 0;
}
