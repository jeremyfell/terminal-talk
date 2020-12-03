Terminal Talk
==============
A simplified implementation of the Unix talk program, to allow messaging between two users on the same local network.

To use, first open the repo and run `make` to build the program.

To run, use `./terminal-talk <user-port> <recipient> <recipient-port>`
- `<user-port>` is the port # for you to receive messages on
- `<recipient>` is the other user's hostname
- `<recipient-port>` is the port the other user is receiving messages on.

e.g. Run `./terminal-talk 7000 userB@machine2 8000`, while the other user runs `./terminal-talk 8000 userA@machine1 7000`

Entering any message in the terminal will be sent to the other user, and received messages will be printed out. To end the connection, simply enter a `!` on the command line.
