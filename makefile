all:
	gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror terminal-talk.c control.c threadsafelist.c list.c receiver.c sender.c input.c output.c  -lpthread -o terminal-talk

clean:
	rm terminal-talk
