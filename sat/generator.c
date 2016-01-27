/* 3SAT instance generator G2(n,m) */
/* created by M.Motoki */
/*
 * edited by:
 * Tomas Nesrovnal
 * nesro@nesro.cz
 * https://github.com/nesro/nesrotom-paa-2015
 * 3SAT GA
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

int **v; /* variable number in each clause */
int **lit; /* literal type in each clause */
int *clause_size; /* size of clause */
int *t; /* solution for formula generator */
int *weights;

/* memory allocation to matrixes s.t. n variables m clauses with at most size k */
static void sat_alloc(int n, int m, int k) {
	int i;

	v = malloc(m * k * sizeof(int));
	assert(v != NULL);
	for (i = 0; i < m; i++) {
		v[i] = malloc(k * sizeof(int));
		assert(v[i] != NULL);
	}

	lit = malloc(m * k * sizeof(int));
	assert(lit != NULL);

	for (i = 0; i < m; i++) {
		lit[i] = malloc(k * sizeof(int));
		assert(lit[i] != NULL);
	}

	clause_size = malloc(m * sizeof(int));
	assert(clause_size != NULL);

	t = malloc(n * sizeof(int));
	assert(t != NULL);
}

/* generate weights line at random with modulus */
/* generate specific count of variable weigthts at random */
static void genWeights(int mod, int count) {
	for (int i = 0; i < count; i++) {
		weights[i] = rand() % mod;
	}
}

/* generate 3CNF with at least 1 solution t*/
/* generate 3CNF with at least 1 solution (1^n) */
static void gen_unique_instance(int n, int m) {
	for (int i = 0; i < m; i++) {
		clause_size[i] = 3;
		do {
			for (int h = 0; h < 3; h++) {
				v[i][h] = rand() % n;
				lit[i][h] = rand() % 2;
			}
		} while ((v[i][0] == v[i][1]) || (v[i][1] == v[i][2])
				|| (v[i][2] == v[i][0])
				|| (lit[i][0] != t[v[i][0]] && lit[i][1] != t[v[i][1]]
						&& lit[i][2] != t[v[i][2]]));
	}
}

/* in DIMACS format with Weights for JCOP */
static void write_sat(FILE *fp, int n, int m) {
	/* config line */
	fprintf(fp, "p cnf %d %d\n", n, m);

	/* weights */
	fprintf(fp, "w");
	for (int i = 0; i < n; i++) {
		fprintf(fp, " %d", weights[i]);
	}
	fprintf(fp, "\n");

	for (int j = 0; j < m; j++) {
		for (int h = 0; h < clause_size[j]; h++) {
			if (!lit[j][h]) {
				fprintf(fp, "-");
			}

			fprintf(fp, "%d ", v[j][h] + 1);
		}

		fprintf(fp, "0\n");
	}
}

static void shuffle_sat(int m, int k) {
	int tmp_clause_size;
	int tmp_v;
	int tmp_lit;
	int tmp;

	for (int j = m - 1; j > 0; j--) {
		tmp = rand() % (j + 1);
		tmp_clause_size = clause_size[j];
		clause_size[j] = clause_size[tmp];
		clause_size[tmp] = tmp_clause_size;
		for (int h = 0; h < k; h++) {
			tmp_v = v[j][h];
			tmp_lit = lit[j][h];
			v[j][h] = v[tmp][h];
			lit[j][h] = lit[tmp][h];
			v[tmp][h] = tmp_v;
			lit[tmp][h] = tmp_lit;
		}
	}
}

int main(int argc, char *argv[]) {
	int n; /* # of variables */
	int m; /* # of clauses */
	int mod; /* weight modulus */
	int i, h; /* loop counter */
	int knownSolutionCost = 0;
	int maxCost = 0;
//	time_t ts;

	if (argc == 4) {
		n = atoi(argv[1]);
		m = atoi(argv[2]);
		mod = atoi(argv[3]);
	} else {
		fprintf(stderr, "%s #variables #clauses mod\n", argv[0]);
		exit(0);
	}

//	ts = time(NULL);
//	srand((unsigned int) ts);

	/* random numbers are useless */
	srand(0);

	sat_alloc(n, m, 3);

	for (i = 0; i < n; i++) {
		t[i] = rand() % 2;
	}

	gen_unique_instance(n, m - n);
	for (i = m - n; i < m; i++) {
		clause_size[i] = 3;
		do {
			for (h = 0; h < 3; h++) {
				v[i][h] = rand() % n;
				lit[i][h] = !t[v[i][h]];
			}
			h = rand() % 3;
			v[i][h] = i - m + n;
			lit[i][h] = t[i - m + n];
		} while ((v[i][0] == v[i][1]) || (v[i][1] == v[i][2])
				|| (v[i][2] == v[i][0]));
	}
	shuffle_sat(m, 3);
	/* allocate array and generate weights */
	weights = malloc(sizeof(int) * n);
	assert(weights != NULL);
	genWeights(mod, n);

	/* the important part */
	write_sat(stdout, n, m);

	/* some extra info. yay. */
	fprintf(stdout, "c instance by G2\nc solution = ");
	for (i = 0; i < n; i++) {
		fprintf(stdout, "%d", t[i]);
		maxCost += weights[i];
		if (t[i] == 1) {
			knownSolutionCost += weights[i];
		}
	}
	fprintf(stdout, "\n");
	fprintf(stdout, "c known solution cost is %d \n", knownSolutionCost);
	fprintf(stdout, "c max cost is %d \n", maxCost);
	fprintf(stdout, "c known + max is %d \n", knownSolutionCost + maxCost);

	free(weights);

	return (EXIT_SUCCESS);
}
