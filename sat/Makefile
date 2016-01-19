CC=gcc
CFLAGS=-Wall -Wextra -Wshadow -Wstrict-overflow -fno-strict-aliasing -Werror -pedantic
LIBS=-fopenmp -lm

all:
	$(CC) $(CFLAGS) -Og -ggdb3 -std=c99 main.c -o main $(CLIBS)
	$(CC) $(CFLAGS) -Og -ggdb3 -std=c99 generator.c -o generator

#fast:
	#gcc -Wall -Wextra -Werror -pedantic -Ofast -std=c99 main.c -o main -fopenmp -lm

clean:
	rm main generator
