all:
	gcc -Wall -Wextra -Werror -pedantic -Og -ggdb3 -std=c99 main.c -o main -fopenmp -lm

fast:
	gcc -Wall -Wextra -Werror -pedantic -Ofast -std=c99 main.c -o main -fopenmp -lm

clean:
	rm main *.plot *.pdf
