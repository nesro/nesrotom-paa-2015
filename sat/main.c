/*
 * Tomas Nesrovnal
 * nesro@nesro.cz
 * https://github.com/nesro/nesrotom-paa-2015
 * 3SAT GA
 */

/******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <omp.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

/******************************************************************************/

typedef struct sat {
	uint32_t vars_cnt;
	uint32_t clauses_cnt;

	uint32_t *weights;
	int32_t **clauses;
} sat_t;

typedef struct individual {
	sat_t *sat;
	bool *ch;
	uint32_t chromosome_len;
	uint32_t fitness;
} ind_t;

typedef struct population {
	sat_t *sat;
	uint32_t inds_cnt;
	ind_t **inds;
} population_t;

/******************************************************************************/

inline void swap_uint32(uint32_t *a, uint32_t *b) {
	uint32_t *tmp = a;
	a = b;
	b = tmp;
}

inline void swap_bool(bool *a, bool *b) {
	bool *tmp = a;
	a = b;
	b = tmp;
}

inline int max(int a, int b) {
	if (a > b) {
		return (a);
	} else {
		return (b);
	}
}

inline int min(int a, int b) {
	if (a < b) {
		return (a);
	} else {
		return (b);
	}
}

/******************************************************************************/

static void sat_free(sat_t *sat) {
	if (sat) {
		free(sat->weights);
		sat->weights = NULL;

		if (sat->clauses) {
			for (uint32_t i = 0; i < sat->clauses_cnt; i++) {
				free(sat->clauses[i]);
				sat->clauses[i] = NULL;
			}
		}

		free(sat->clauses);
		sat->clauses = NULL;
	}

	free(sat);
	sat = NULL;
}

/* http://people.sc.fsu.edu/~jburkardt/data/cnf/cnf.html */
static sat_t *sat_load() {
	sat_t *sat = NULL;

	sat = calloc(1, sizeof(sat_t));
	assert(sat != NULL);

	if (scanf("p cnf %" SCNu32 " %" SCNu32, &sat->vars_cnt, &sat->clauses_cnt)
			!= 2) {
		fprintf(stderr, "scanf p cnf %%d %%d has failed\n");
		goto error;
	}

	sat->weights = malloc(sizeof(uint32_t) * sat->vars_cnt);

	char c;
	if (scanf("%c", &c) != 1) {
		fprintf(stderr, "reading w failed\n");
		goto error;
	}
	if ((c = getchar()) != 'w') {
		fprintf(stderr, "reading w failed its -%c-\n", c);
		goto error;
	}

	for (uint32_t i = 0; i < sat->vars_cnt; i++) {
		int res;
		if ((res = scanf("%" SCNu32, &sat->weights[i])) != 1) {
			fprintf(stderr, "reading weight %d has failed, scanf=%d\n", i, res);
			goto error;
		}
	}

	sat->clauses = malloc(sizeof(int8_t *) * sat->clauses_cnt);
	assert(sat->clauses);
	for (uint32_t i = 0; i < sat->clauses_cnt; i++) {
		sat->clauses[i] = malloc(sizeof(int8_t *) * (sat->clauses_cnt + 1));
		assert(sat->clauses[i]);

		for (uint32_t j = 0; j < 3; j++) {
			if (scanf("%" SCNd32, &sat->clauses[i][j]) != 1) {
				fprintf(stderr, "reading clause [%d][%d] has failed\n", i, j);
				goto error;
			}
		}

		if (scanf("%" SCNd32, &sat->clauses[i][sat->vars_cnt]) != 1) {
			fprintf(stderr, "reading terminating clause [%d][] has failed\n",
					i);
			goto error;
		}
		if (sat->clauses[i][sat->vars_cnt] != 0) {
			fprintf(stderr, "terminating clause [%d][] is not 0 but %d\n", i,
					sat->clauses[i][sat->vars_cnt]);
			goto error;
		}
	}

	return (sat);

	error: //
	sat_free(sat);
	return (NULL);
}

void ind_free(ind_t *individual) {
	if (individual) {
		free(individual->ch);
		individual->ch = NULL;
	}
	free(individual);
	individual = NULL;
}

uint32_t formula_weight(sat_t *sat, const bool *chromosome) {
	uint32_t weight = 0;
	for (uint32_t i = 0; i < sat->vars_cnt; i++) {
		if (chromosome[i]) {
			weight += sat->weights[i];
		}
	}

	return (weight);
}

bool is_clause_true(sat_t *sat, bool *genome, int clause_id) {
	for (uint32_t i = 0; i < 3; i++) {
		int32_t val = sat->clauses[clause_id][i];
		if (val > 0) {
			if (genome[val - 1]) {
				return (true);
			}
		} else {
			if (!genome[abs(val) - 1]) {
				return (true);
			}
		}
	}

	return (false);
}

void ind_compute_fitness(ind_t *individual) {
	uint32_t true_cnt = 0;

	for (uint32_t i = 0; i < individual->sat->clauses_cnt; i++) {
		if (is_clause_true(individual->sat, individual->ch, i)) {
			true_cnt++;
		}
	}

	individual->fitness = true_cnt;

	if (true_cnt == individual->sat->clauses_cnt) {
//		printf("clausole is true!\n");
		individual->fitness += formula_weight(individual->sat, individual->ch);
	}
}

ind_t *ind_init(sat_t *sat) {
	ind_t *individual = NULL;

	individual = malloc(sizeof(ind_t));
	assert(individual != NULL);

	individual->sat = sat;
	individual->chromosome_len = sat->vars_cnt;

	individual->ch = malloc(sizeof(bool) * individual->chromosome_len);
	assert(individual->ch != NULL);

	/* randomize chromosome */
	for (uint32_t i = 0; i < individual->chromosome_len; i++) {
		individual->ch[i] = rand() % 2;
	}

	ind_compute_fitness(individual);

	return (individual);

//	error: //
//	individual_free(individual);
//	return (NULL);
}

void population_free(population_t *population) {
	if (population != NULL) {
		for (uint32_t i = 0; i < population->inds_cnt; i++) {
			ind_free(population->inds[i]);
		}
		free(population->inds);
	}
	free(population);
	population = NULL;
}

population_t *pop_init(sat_t *sat, uint32_t size) {
	population_t *population = NULL;

	population = calloc(1, sizeof(population_t));
	assert(population);

	population->sat = sat;
	population->inds_cnt = size;

	population->inds = calloc(size, sizeof(ind_t));
	if (!population->inds) {
		fprintf(stderr, "calloc individuals\n");
		goto error;
	}

	for (uint32_t i = 0; size > i; i++) {
		population->inds[i] = ind_init(sat);
		assert(population->inds[i]);
	}

	return (population);

	error: //
	population_free(population);
	return (NULL);
}

/* sa = solution array */
ind_t* ga_select(population_t *population) {
	const int in_tournament = 50;
	ind_t **sa = population->inds;
	ind_t *selected[in_tournament];
	int i;
	int best = 0;

	for (i = 0; i < in_tournament; i++) {
		selected[i] = sa[rand() % population->inds_cnt];
	}

	for (i = 1; i < in_tournament; i++) {
		if (selected[i]->fitness > selected[best]->fitness) {
			best = i;
		}
	}

	return (selected[best]);
}

void ga_cross_2p(bool *a, bool *b, uint32_t n) {
	uint32_t cross_point1 = rand() % n;
	uint32_t cross_point2 = rand() % n;

	if (cross_point1 > cross_point2) {
		swap_uint32(&cross_point1, &cross_point2);
	}

	for (uint32_t i = 0; i < n; i++) {
		if (i < cross_point1 && i > cross_point2) {
			swap_bool(&a[i], &b[i]);
		}
	}
}

void ga_mutate(bool *ch, uint32_t n) {
	int muation = 200;
	for (uint32_t i = 0; i < n; i++) {
		if (rand() % 1000 <= muation) {
			ch[i] = ch[i] ? 0 : 1;
		}
	}
}

int ga_fitness_cmp(const void *a, const void *b) {
	ind_t *sa = *(ind_t **) a;
	ind_t *sb = *(ind_t **) b;

	return (sb->fitness - sa->fitness);
}

void ind_print(ind_t *ind) {
	printf("f=%u\tch=", ind->fitness);
	for (uint32_t i = 0; i < ind->chromosome_len; i++) {
		printf("%u", ind->ch[i]);
	}
	printf("\n");
}

void sat_simulated_evolution(sat_t *sat, uint32_t pop_size) {
	population_t *pop = NULL;
	population_t *pop_next = NULL;
	population_t *pop_swap = NULL;

	pop = pop_init(sat, pop_size);
	pop_next = pop_init(sat, pop_size);

	uint32_t ic = sat->vars_cnt;
	uint32_t favg; /* average fitness */

	ind_t *ind_best = ind_init(sat);

	for (uint32_t gen = 0; gen < 10000; gen++) {
		for (uint32_t i = 0; i < pop->inds_cnt; i += 2) {

			memcpy(pop_next->inds[i + 0]->ch, ga_select(pop)->ch,
					sizeof(bool) * ic);
			memcpy(pop_next->inds[i + 1]->ch, ga_select(pop)->ch,
					sizeof(bool) * ic);

			ga_cross_2p(pop_next->inds[i + 0]->ch, pop_next->inds[i + 1]->ch,
					ic);

			ga_mutate(pop_next->inds[i + 0]->ch, ic);
			ga_mutate(pop_next->inds[i + 1]->ch, ic);

			ind_compute_fitness(pop_next->inds[i + 0]);
			ind_compute_fitness(pop_next->inds[i + 1]);
		}
		pop_swap = pop;
		pop = pop_next;
		pop_next = pop_swap;

		qsort(pop->inds, pop->inds_cnt, sizeof(ind_t **), ga_fitness_cmp);

		/* compute average fitness */
		favg = 0;
		for (uint32_t i = 0; i < pop->inds_cnt; i++) {
			favg += pop->inds[i]->fitness;
		}
		favg /= pop->inds_cnt;

		fprintf(stdout, "gen=%u avg=%u best ind:", gen, favg);
		ind_print(pop->inds[0]);

		if (ind_best->fitness < pop->inds[0]->fitness) {
			memcpy(ind_best->ch, pop->inds[0]->ch, sizeof(bool) * ic);
		}
	}

	fprintf(stdout, "total best: ");
	ind_print(ind_best);

//	printf("best ind: ");
//	ind_print(pop->inds[0]);

	ind_free(ind_best);
	population_free(pop);
	population_free(pop_next);
}

void sat_print(sat_t *sat) {
	printf("clauses_cnt=%u, vars_cnt=%u\n", sat->clauses_cnt, sat->vars_cnt);

	for (uint32_t i = 0; i < sat->clauses_cnt; i++) {
		printf("clauses[%u]: ", i);
		for (uint32_t j = 0; j < 3; j++) {
			printf("[%2" PRId32"], ", sat->clauses[i][j]);
		}
		printf("\n");
	}

	for (uint32_t i = 0; i < sat->vars_cnt; i++) {
		printf("weights[%u] = %u\n", i, sat->weights[i]);
	}
}
/******************************************************************************/

int main(void) {
	sat_t *sat;

	srand(1);

	sat = sat_load();
	//sat_print(sat);

	sat_simulated_evolution(sat, 100);

	sat_free(sat);

	return (EXIT_SUCCESS);
}
