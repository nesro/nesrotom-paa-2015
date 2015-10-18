/*
 * Tomas Nesrovnal
 * nesro@nesro.cz
 * https://github.com/nesro/nesrotom-paa-2015
 */

/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <omp.h>
#include <ctype.h>
#include <unistd.h>

/******************************************************************************/

#define MAX_ITEMS 40

/******************************************************************************/

typedef struct knapsack_item {
	int id;
	double weight;
	double cost;
} knapsack_item_t;

typedef struct knapsack_solution {
	int items[MAX_ITEMS];
	double weight;
	double cost;
} knapsack_solution_t;

typedef struct knapsack {
	int id;
	int n; /* how many items */
	int cap; /* capacity (how much weight we can hold) */
	knapsack_item_t items[MAX_ITEMS]; /* available items */
	knapsack_solution_t solution;
} knapsack_t;

typedef enum knapsack_method {
	UNKNOWN_METHOD, BRUTEFORCE, HEURISTIC
} knapsack_method_t;

typedef enum knapsnack_heuristic {
	UNKNOWN_HEURISTIC = 0, RATIO_CW = 1, RATIO_WC = 2, COST = 3, WEIGHT = 4
} knapsnack_heuristic_t;

/******************************************************************************/

int knapsack_load(knapsack_t *k) {
	int i;

	if (scanf("%d", &k->id) == EOF) {
		return (0);
	}

	if (scanf("%d", &k->n) != 1) {
		return (0);
	}
	assert(k->n <= MAX_ITEMS);

	if (scanf("%d", &k->cap) != 1) {
		return (0);
	}

	for (i = 0; i < k->n; i++) {
		if (scanf("%lf", &k->items[i].weight) != 1) {
			return (0);
		}
		if (scanf("%lf", &k->items[i].cost) != 1) {
			return (0);
		}
		k->items[i].id = i;
	}

	return (1);
}

void knapsack_solve_bruteforce_inner(knapsack_t *k, int n,
		knapsack_solution_t *s/*olution*/) {

	if (n == 0 && s->weight <= k->cap && s->cost >= k->solution.cost) {
		memcpy(&k->solution, s, sizeof(knapsack_solution_t));
		return;
	}

	if (n == 0) {
		return;
	}

	n--;

	knapsack_solve_bruteforce_inner(k, n, s);

	if (s->weight + k->items[n].weight > k->cap) {
		return;
	}

	s->items[n] = 1;
	s->weight += k->items[n].weight;
	s->cost += k->items[n].cost;
	knapsack_solve_bruteforce_inner(k, n, s);
	s->items[n] = 0;
	s->weight -= k->items[n].weight;
	s->cost -= k->items[n].cost;
}

void knapsack_solve_bruteforce(knapsack_t *k) {
	knapsack_solution_t s = { { 0 }, 0, 0 };
	memcpy(&k->solution, &s, sizeof(knapsack_solution_t));
	knapsack_solve_bruteforce_inner(k, k->n, &s);
}

int knapsack_item_compare_ratio_cw(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;

	return ((ia->cost / ia->weight) - (ib->cost / ib->weight));
}

int knapsack_item_compare_ratio_wc(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;

	return ((ia->weight / ia->cost) - (ib->weight / ib->cost));
}

int knapsack_item_compare_cost(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;

	return (ia->cost - ib->cost);
}

int knapsack_item_compare_weight(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;

	return (ia->weight - ib->weight);
}

void knapsack_solve_heuristic(knapsack_t *k, knapsnack_heuristic_t h) {
	int i;
	int (*qsort_method)(const void *, const void *);

	switch (h) {
	case RATIO_CW:
		qsort_method = knapsack_item_compare_ratio_cw;
		break;
	case RATIO_WC:
		qsort_method = knapsack_item_compare_ratio_wc;
		break;
	case COST:
		qsort_method = knapsack_item_compare_cost;
		break;
	case WEIGHT:
		qsort_method = knapsack_item_compare_weight;
		break;
	case UNKNOWN_METHOD:
	default:
		fprintf(stderr, "Unknown heurstic method: %d.\n", h);
		assert(0);
		break;
	}

	qsort(k->items, k->n, sizeof(knapsack_item_t), qsort_method);
	memset(&k->solution, 0, sizeof(knapsack_solution_t));

	for (i = k->n - 1; i >= 0; i--) {
		if (k->solution.weight + k->items[i].weight < k->cap) {
			k->solution.weight += k->items[i].weight;
			k->solution.cost += k->items[i].cost;
			k->solution.items[k->items[i].id] = 1;
		}
	}
}

void knapsack_print(knapsack_t *k) {
	int i;

	printf("%d %d %lf ", k->id, k->n, k->solution.cost);
	for (i = 0; i < k->n; i++) {
		printf(" %d", k->solution.items[i]);
	}
	printf("\n");
}

/******************************************************************************/

int main(int argc, char *argv[]) {
	knapsack_t k;
	knapsack_method_t method = UNKNOWN_METHOD;
	int heuristic = UNKNOWN_HEURISTIC;
	int c;
	int i;
	int repeat = 1;
	double time_start = 0;
	double time_end = 0;
	int time_show = 0;

	while ((c = getopt(argc, argv, "bh:r:t")) != -1) {
		switch (c) {
		case 'b':
			method = BRUTEFORCE;
			break;
		case 'h':
			method = HEURISTIC;
			heuristic = atoi(optarg);
			break;
		case 'r':
			repeat = atoi(optarg);
			break;
		case 't':
			time_show = 1;
			break;
		case '?':
			fprintf(stderr, "unknown opt omg\n");
			return (EXIT_FAILURE);
			break;
		default:
			abort();
			break;
		}
	}

	if (time_show) {
		time_start = omp_get_wtime();
	}

	while (knapsack_load(&k)) {
		for (i = 0; i < repeat; i++) {
			switch (method) {
			case BRUTEFORCE:
				knapsack_solve_bruteforce(&k);
				break;
			case HEURISTIC:
				knapsack_solve_heuristic(&k, heuristic);
				break;
			case UNKNOWN_METHOD:
				fprintf(stderr, "select method pls\n");
				return (EXIT_FAILURE);
				break;
			default:
				abort();
				break;
			}
		}
		if (!time_show) {
			knapsack_print(&k);
		}
	}

	if (time_show) {
		time_end = omp_get_wtime();
		printf("%lf\n", time_end - time_start);
	}

	return (EXIT_SUCCESS);
}
