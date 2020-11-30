// Defines several constants and functions to control the threads of the program
#ifndef _CONTROL_H_
#define _CONTROL_H_

#define TERMINATE "!\n"
#define MESSAGE_MAX_SIZE 512
#define HOSTNAME_MAX_SIZE 256

// Wait for the program to be terminated by the local or remote user
void Control_waitForTermination(void);

// Signal that the program should be terminated
void Control_signalTermination(void);

// Cleans up internal variables
void Control_cleanup(void);

#endif
