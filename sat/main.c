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

	uint8_t *weights;
	int8_t **clauses;
} sat_t;

typedef struct individual {
	sat_t *sat;
	bool *chromosome;
	uint32_t chromosome_len;
	uint32_t fitness;
} individual_t;

typedef struct population {
	sat_t *sat;
	uint32_t *individuals_cnt;
	individual_t *individuals;
} population_t;

/******************************************************************************/

static void sat_free(sat_t *sat) {
	if (sat) {
		free(sat->weights);
		sat->weights = NULL;

		if (sat->clauses) {
			for (int i = 0; i < sat->vars_cnt; i++) {
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

	if (scanf("p cnf %d %d", &sat->vars_cnt, &sat->clauses_cnt) != 2) {
		fprintf(stderr, "scanf p cnf %d %d has failed\n");
		goto error;
	}

	sat->weights = malloc(sizeof(uint8_t) * sat->vars_cnt);
	scanf("w");
	for (uint32_t i = 0; i < sat->vars_cnt; i++) {
		if (scanf("%d", &sat->weights[i]) != 1) {
			fprintf(stderr, "reading weight %d has failed\n", i);
			goto error;
		}
	}

	sat->clauses = malloc(sizeof(int8_t *) * sat->clauses_cnt);
	assert(sat->clauses);
	for (uint32_t i = 0; i < sat->clauses_cnt; i++) {
		sat->clauses[i] = malloc(sizeof(int8_t *) * (sat->vars_cnt + 1));
		assert(sat->clauses[i]);

		for (uint32_t j = 0; j < sat->vars_cnt; j++) {
			if (scanf("%d", &sat->clauses[i][j]) != 1) {
				fprintf(stderr, "reading clause [%d][%d] has failed\n", i, j);
				goto error;
			}
		}

		if (scanf("%d", &sat->clauses[i][sat->vars_cnt]) != 1) {
			fprintf(stderr, "reading terminating clause [%d][] has failed\n",
					i);
			goto error;
		}
		if (sat->clauses[i][sat->vars_cnt] != 0) {
			fprintf(stderr, "terminating clause [%d][] is not 0\n", i);
			goto error;
		}
	}

	return (sat);

	error: //
	sat_free(sat);
	return (NULL);
}

void individual_free(individual_t *individual) {
	if (individual) {
		free(individual->chromosome);
		individual->chromosome = NULL;
	}
	free(individual);
	individual = NULL;
}

individual_t *individual_init(sat_t *sat) {
	individual_t *individual = NULL;

	individual = malloc(sizeof(individual_t));
	assert(individual != NULL);

	individual->sat = sat;
	individual->chromosome_len = sat->vars_cnt;

	individual->chromosome = malloc(sizeof(bool) * individual->chromosome_len);
	assert(individual->chromosome != NULL);

	/* randomize chromosome */
	for (int i = 0; i < individual->chromosome_len; i++) {
		individual->chromosome[i] = rand() % 2;
	}

	individual->fitness = individual_compute_fitness(individual);

	return (individual);

	error: //
	individual_free(individual);
	return (NULL);
}

bool is_clause_true(sat_t *sat, bool *genome, int clause_id) {
	int val, is_true = 0;
	for (int i = 0; sat->clauses_cnt > i; i++) {
		val = sat->clauses[clause_id][i];
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

uint32_t formula_weight(sat_t *sat, const bool *chromosome) {
	uint32_t weight = 0;
	for (uint32_t i = 0; i < sat->vars_cnt; i++) {
		if (chromosome[i]) {
			weight += sat->weights[i];
		}
	}

	return (weight);
}

void individual_compute_fitness(individual_t *individual) {
	uint32_t true_cnt = 0;

	for (uint32_t i = 0; i < individual->sat->clauses_cnt; i++) {
		if (is_clause_true(individual->sat, individual->chromosome, i)) {
			true_cnt++;
		}
	}

	individual->fitness = true_cnt;

	if (true_cnt == individual->sat->clauses_cnt) {
		individual->fitness += formula_weight(individual->sat,
				individual->chromosome);
	}
}

void population_free(population_t *population) {
	if (population != NULL) {

	}
	free(population);
	population = NULL;
}

population_t *population_init(sat_t *sat, uint32_t size) {
	population_t *population = NULL;

	population = calloc(1, sizeof(population_t));
	assert(population);

	population->sat = sat;
	population->individuals_cnt = size;

	population->individuals = calloc(size, sizeof(individual_t));
	for (uint32_t i = 0; size > i; i++) {
		population->individuals[i] = individual_init(sat);
		assert(population->individuals[i] != NULL);
	}

	return (population);

	error: //
	population_free(population);
	return (NULL);
}

void simulated_evolution(sat_t *sat, uint32_t poulation_size) {
	population_t *population = NULL;

	population = population_init(sat, poulation_size);

	for (uint32_t gen = 0; gen < 100; gen++) {
		for (uint32_t ind = 0; ind < population->individuals_cnt; ind++) {

		}
	}

	population_free(population);
}

/******************************************************************************/

int main(void) {
	sat_t *sat;

	sat = sat_load();

	sat_free(sat);

	return (EXIT_SUCCESS);
}
