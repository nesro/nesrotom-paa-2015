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

#define MAX_ITEMS 50

/******************************************************************************/

typedef struct knapsack_result {
	double time;
	double error_relative;
	double error_maximal;
} knapsnack_result_t;

typedef struct knapsack_item {
	int id;
	double weight;
	double cost;
} knapsack_item_t;

typedef struct knapsack_solution {
	int items[MAX_ITEMS];
	double weight;
	double cost;
	double cost_best; /* obsolete */
} knapsack_solution_t;

typedef struct knapsack {
	int id;
	int n; /* how many items */
	int cap; /* capacity (how much weight we can hold) */
	knapsack_item_t items[MAX_ITEMS]; /* available items */
	knapsack_solution_t solution;
	double cost_best;
} knapsack_t;

typedef enum knapsack_method {
	UNKNOWN_METHOD, BRUTEFORCE, HEURISTIC
} knapsack_method_t;

typedef enum knapsnack_heuristic {
	UNKNOWN_HEURISTIC = 0, RATIO_CW = 1, COST = 2, WEIGHT = 3
} knapsnack_heuristic_t;

/******************************************************************************/

int knapsack_load(knapsack_t *k, int pass_best) {
	int i;

	k->solution.cost = 0;
	k->solution.cost_best = 0;
	k->solution.weight = 0;

	if (pass_best) {
		if (scanf("%lf", &(k->cost_best)) == EOF) {
			return (0);
		}
		k->solution.cost_best = k->cost_best; /* obsolete */
	}

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

int knapsack_item_compare_ratio_cw(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;
	double res = ((ia->cost / ia->weight) - (ib->cost / ib->weight));
	if (res < 0) {
		return (-1);
	} else if (res > 0) {
		return (1);
	} else {
		return (0);
	}
}

int knapsack_item_compare_cost(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;
	double res = (ia->cost - ib->cost);
	if (res < 0) {
		return (-1);
	} else if (res > 0) {
		return (1);
	} else {
		return (0);
	}
}

/* int lc = 0;
 int uc = 0; */

void knapsack_solve_bruteforce_inner(knapsack_t *k, int n,
		knapsack_solution_t *s/*olution*/, int cut,
		knapsack_item_t *r/*emaining*/) {

	if (n == 0) {
		if (s->weight <= k->cap && s->cost >= k->solution.cost) {
			memcpy(&k->solution, s, sizeof(knapsack_solution_t));
		}
		return;
	}

	n--;

	/* lower cut - even if we add all remaining items, the cost wouldn't
	 * be enough to be better than the best one we have found */
	if (cut && k->solution.cost > r[n].cost) {
		/* lc++; */
		return;
	}

	knapsack_solve_bruteforce_inner(k, n, s, cut, r);

	/* upper cut - if the added item is too heavy for the knapsnack */
	if (cut && s->weight + k->items[n].weight > k->cap) {
		/* uc++; */
		return;
	}

	s->items[n] = 1;
	s->weight += k->items[n].weight;
	s->cost += k->items[n].cost;
	knapsack_solve_bruteforce_inner(k, n, s, cut, r);
	s->items[n] = 0;
	s->weight -= k->items[n].weight;
	s->cost -= k->items[n].cost;
}

void knapsack_solve_bruteforce(knapsack_t *k, int optimize) {
	knapsack_solution_t s = { { 0 }, 0, 0, 0 };
	knapsack_item_t remaining[50]; /* sum of remaining */
	int i;

	memcpy(&k->solution, &s, sizeof(knapsack_solution_t));

	if (optimize) {
		qsort(k->items, k->n, sizeof(knapsack_item_t),
				knapsack_item_compare_ratio_cw);

		remaining[k->n - 1].cost = k->items[k->n - 1].cost;
		remaining[k->n - 1].weight = k->items[k->n - 1].weight;
		for (i = k->n - 2; i >= 0; i--) {
			remaining[i].cost = remaining[i - 1].cost + k->items[i].cost;
			remaining[i].weight = remaining[i - 1].weight + k->items[i].weight;
		}
	}

	knapsack_solve_bruteforce_inner(k, k->n, &s, optimize, remaining);
}

int knapsack_item_compare_weight(const void *a, const void *b) {
	knapsack_item_t *ia = (knapsack_item_t *) a;
	knapsack_item_t *ib = (knapsack_item_t *) b;
	double res = (ia->weight - ib->weight);
	if (res < 0) {
		return (-1);
	} else if (res > 0) {
		return (1);
	} else {
		return (0);
	}
}

void knapsack_solve_heuristic(knapsack_t *k, knapsnack_heuristic_t h) {
	int i;
	int (*qsort_method)(const void *, const void *);

	switch (h) {
	case RATIO_CW:
		qsort_method = knapsack_item_compare_ratio_cw;
		break;
	case COST:
		qsort_method = knapsack_item_compare_cost;
		break;
	case WEIGHT:
		qsort_method = knapsack_item_compare_weight;
		break;
	case UNKNOWN_METHOD:
	default:
		fprintf(stderr, "Unknown heuristic method: %d.\n", h);
		assert(0);
		break;
	}

	qsort(k->items, k->n, sizeof(knapsack_item_t), qsort_method);
	k->solution.weight = 0;
	k->solution.cost = 0;

	for (i = k->n - 1; i >= 0; i--) {
		if (k->solution.weight + k->items[i].weight <= k->cap) {
			k->solution.weight += k->items[i].weight;
			k->solution.cost += k->items[i].cost;
			k->solution.items[k->items[i].id] = 1;
		} else {
			k->solution.items[k->items[i].id] = 0;
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
	knapsnack_result_t result = { 0, 0, 0 };
	int pass_best = 0;
	int bruteforce_cut = 0;
	double relative;
	double lines = 0;

	while ((c = getopt(argc, argv, "b:h:r:tp")) != -1) {
		switch (c) {
		case 'b':
			method = BRUTEFORCE;
			bruteforce_cut = atoi(optarg);
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
		case 'p':
			pass_best = 1;
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

	result.time = omp_get_wtime();
	result.error_relative = 0.0;
	result.error_maximal = 0.0;

	while (knapsack_load(&k, pass_best)) {
		lines++;
		for (i = 0; i < repeat; i++) {
			switch (method) {
			case BRUTEFORCE:
				knapsack_solve_bruteforce(&k, bruteforce_cut);
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

		relative = (k.solution.cost_best - k.solution.cost)
				/ k.solution.cost_best;

		if (relative > result.error_maximal) {
			result.error_maximal = relative;
		}

		result.error_relative += relative;

		if (!time_show) {
			knapsack_print(&k);
		}
	}

	result.time = omp_get_wtime() - result.time;

	result.error_relative = result.error_relative / lines;
	printf("%lf_%lf_%lf\n", result.error_relative, result.error_maximal,
			result.time);

	if (time_show) {
		time_end = omp_get_wtime();
		printf("%lf\n", (time_end - time_start) / (double) repeat);
	}

	/* fprintf(stderr, "lower cuts=%d, upper cuts=%d\n", lc, uc); */

	return (EXIT_SUCCESS);
}
