CC= gcc
CFLAGS= -Wall -Wextra -std=c11 -lm

tictactoe: tictactoe.c
	$(CC) $(CFLAGS) -o tictactoe tictactoe.c

clean:
	rm -f tictactoe
