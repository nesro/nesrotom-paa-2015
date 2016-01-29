CC=gcc
CFLAGS=-std=c99 -Wall -Wpedantic -Wextra -Wshadow -Werror -Wstrict-overflow -fno-strict-aliasing  
CLIBS=-fopenmp -lm

all:
	$(CC) $(CFLAGS) -Ofast main.c -o main $(CLIBS)

debug:
	$(CC) $(CFLAGS) -Og -ggdb3 main.c -o main $(CLIBS)
	$(CC) $(CFLAGS) -Og -ggdb3 generator.c -o generator

clean:
	rm main generator
