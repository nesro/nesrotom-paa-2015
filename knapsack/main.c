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
#define MAX_CAPACITY 1000
#define MAX_COST 8000

/******************************************************************************/

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

typedef struct knapsack_result {
	double time;
	double error_relative;
	double error_maximal;
} knapsnack_result_t;

typedef struct knapsack_item {
	int id;
	int weight;
	int cost;
} knapsack_item_t;

typedef struct knapsack_solution {
	int items[MAX_ITEMS];
	int weight;
	int cost;
	int cost_best; /* obsolete */
} knapsack_solution_t;

typedef struct knapsack {
	int id;
	int n; /* how many items */
	int cap; /* capacity (how much weight we can hold) */
	knapsack_item_t items[MAX_ITEMS]; /* available items */
	knapsack_solution_t solution;
	int cost_best;
} knapsack_t;

typedef enum knapsack_method {
	UNKNOWN_METHOD, BRUTEFORCE, HEURISTIC, DYNAMIC, FPTAS
} knapsack_method_t;

typedef enum knapsnack_heuristic {
	UNKNOWN_HEURISTIC = 0, RATIO_CW = 1, COST = 2, WEIGHT = 3
} knapsack_heuristic_t;

typedef enum knapsnack_dynamic {
	UNKNOWN_DYNAMIC = 0, UP = 1, DOWN = 2
} knapsack_dynamic_t;

/******************************************************************************/

int knapsack_load(knapsack_t *k, int pass_best) {
	int i;

	k->solution.cost = 0;
	k->solution.cost_best = 0;
	k->solution.weight = 0;

	if (pass_best) {
		if (scanf("%d", &(k->cost_best)) == EOF) {
			return (0);
		}
		k->solution.cost_best = k->cost_best; /* obsolete */
	}

	if (scanf("%d", &k->id) == EOF) {
		return (0);
	}

	if (scanf("%d", &k->n) != 1) {
		fprintf(stderr, "scanf failed\n");
		return (0);
	}
	assert(k->n <= MAX_ITEMS);

	if (scanf("%d", &k->cap) != 1) {
		return (0);
	}
	assert(k->cap <= MAX_CAPACITY);

	for (i = 0; i < k->n; i++) {
		if (scanf("%d", &k->items[i].weight) != 1) {
			return (0);
		}
		if (scanf("%d", &k->items[i].cost) != 1) {
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

int lc = 0;
int uc = 0;

void knapsack_solve_bruteforce_inner(knapsack_t *k, int n,
		knapsack_solution_t *s/*olution*/, int cut,
		knapsack_item_t *r/*emaining*/) {

	if (0)
		printf("knapsack_solve_bruteforce_inner n=%d\n", n);

	if (n == 0) {
		if (s->weight <= k->cap && s->cost >= k->solution.cost) {
			memcpy(&k->solution, s, sizeof(knapsack_solution_t));
		}
		return;
	}

	n--;

	/* upper cut - if the added item is too heavy for the knapsnack */
	if (cut && s->weight + k->items[n].weight > k->cap) {
		uc++;
		goto without_item;
		return;
	}

	/* lower cut - even if we add all remaining items, the cost wouldn't
	 * be enough to be better than the best one we have found */
	if (0)
		fprintf(stderr, "n=%d, c=%d, r=%d\n", n, k->solution.cost, r[n].cost);
	if (cut && k->solution.cost > r[n].cost) {
		lc++;
		return;
	}

	/* put the n-th item in and recurse */
	s->items[k->items[n].id] = 1;
	s->weight += k->items[n].weight;
	s->cost += k->items[n].cost;
	knapsack_solve_bruteforce_inner(k, n, s, cut, r);
	s->items[k->items[n].id] = 0;
	s->weight -= k->items[n].weight;
	s->cost -= k->items[n].cost;

	without_item:
	/* recurse without the item */
	knapsack_solve_bruteforce_inner(k, n, s, cut, r);

}

void knapsack_solve_bruteforce(knapsack_t *k, int optimize) {
	knapsack_solution_t s = { { 0 }, 0, 0, 0 };
	knapsack_item_t remaining[50]; /* sum of remaining */
	int i;

	memcpy(&k->solution, &s, sizeof(knapsack_solution_t));

	if (optimize) {
		qsort(k->items, k->n, sizeof(knapsack_item_t),
				knapsack_item_compare_ratio_cw);

		remaining[0].cost = k->items[0].cost;
		remaining[0].weight = k->items[0].weight;
		remaining[0].id = 0;
		for (i = 1; i < k->n; i++) {
			remaining[i].cost = remaining[i - 1].cost + k->items[i].cost;
			remaining[i].weight = remaining[i - 1].weight + k->items[i].weight;
			remaining[i].id = 0;
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

int **malloc2d(int size) {
	int **r;

	r = malloc(size * sizeof(int *));
	assert(r);

	return (r);
}

int knapsack_d2(knapsack_t *k, int i, int w) {
	if (i < 0 || w < 0) {
		return (0);
	} else if (k->items[i].weight > w) {
		return (knapsack_d2(k, i - 1, w));
	} else {
		return (max(knapsack_d2(k, i - 1, w),
				knapsack_d2(k, i - 1, w - k->items[i].weight)
						+ k->items[i].cost));
	}
}

void knapsack_solve_dynamic_down(knapsack_t *k) {
	k->solution.cost = knapsack_d2(k, k->n - 1, k->cap);
}

void knapsack_solve_dynamic_up(knapsack_t *k) {
	int i;
	int w;
	int dyntbl[MAX_ITEMS + 1][MAX_CAPACITY + 1];

	for (i = 0; i <= k->n; i++) {
		for (w = 0; w <= k->cap; w++) {
			dyntbl[i][w] = 0;
		}
	}

	for (i = 1; i <= k->n; i++) {
		for (w = 0; w <= k->cap; w++) {
			if (k->items[i - 1].weight <= w) {
				dyntbl[i][w] = max(
						k->items[i - 1].cost
								+ dyntbl[i - 1][w - k->items[i - 1].weight],
						dyntbl[i - 1][w]);
			} else {
				/* over limit, just copy the previous line */
				dyntbl[i][w] = dyntbl[i - 1][w];
			}
		}
	}

	k->solution.cost = dyntbl[k->n][k->cap];

	/*for (w = 0; w <= k->cap; w++) {
	 for (i = 0; i <= k->n; i++) {
	 printf("[%d][%2d]%d\t", i, w, dyntbl[i][w]);
	 }
	 printf("\n");
	 }*/

}

void knapsack_fptas(knapsack_t *k) {
	/*
	 * 2. kolik bytu budu ignorovat: vzorecek
	 * (dvojkovej logarismtus z log2(eps*costMAx/pocetpredmetu)
	 * VELIKOST TABULKY JE Z VYSCALOVANE SUMY
	 * 1. suma vsech VYSCALOVANYCH cen + maximalni cena
	 * 3. kdyz je to <0 tak dej nula
	 * 4. vsechny ceny udelam >> o ten vysledek (abych zmensil pole)
	 * 5. do prvniho radku; arr[0][i] = intMax/2 (ze tam nebyla nejlepsi cena)
	 *    arr[0][0] = 0;
	 * 6. dynamo, jen misto max dat min (hledam nejmensi vahu)
	 * 7. ja pak nevim kde je vysledek, vim  jen, ze je v poslendim radku
	 *    takze prochazim od konce posledni radek a hledam tam hiodnotu
	 *    kteraje mensi ma vahu co se tam jeste vejde
	 *
	 * 8. zjistim, ktery predmety dal (neni potreba)]
	 * 9. na konci tu cenu zase zasunu <<
	 */

	int i;
	int c;
	int dyntbl[MAX_ITEMS + 1][MAX_COST + 1];

	int cost_sum = 0;
	int cost_max = 0;
	for (i = 0; i < k->n; i++) {
		cost_sum += k->items[i].cost;
		cost_max = max(cost_max, k->items[i].cost);
	}

	/* tady vyskaluju cost_sum */

	assert(cost_sum < MAX_COST);

	for (i = 0; i <= k->n; i++) {
		for (c = 0; c <= cost_sum; c++) {
			dyntbl[i][c] = 0;
		}
	}

	for (i = 0; i <= cost_sum; i++) {
		dyntbl[0][i] = 999999;
	}
	dyntbl[0][0] = 0;

	for (i = 1; i <= k->n; i++) {
		for (c = 0; c <= cost_sum; c++) {
			if (k->items[i - 1].cost <= c) {
				dyntbl[i][c] = min(
						k->items[i - 1].weight
								+ dyntbl[i - 1][c - k->items[i - 1].cost],
						dyntbl[i - 1][c]);
			} else {
				/* over limit, just copy the previous line */
				dyntbl[i][c] = dyntbl[i - 1][c];
			}
		}
	}

	for (i = cost_sum; i > 0; i--) {
		if (dyntbl[k->n][i] <= k->cap) {
			k->solution.cost = i;
			break;
		}
	}
	/*
	 int j;
	 int res = 0;
	 for (j = k->n; j > 0; j--) {
	 if (dyntbl[j][i] != dyntbl[j - 1][i]) {
	 i -= k->items[j - 1].cost;
	 res += k->items[j - 1].cost;
	 }
	 }
	 k->solution.cost = res;
	 */
	/*
	 if (0) {
	 for (c = 0; c <= cost_sum; c++) {
	 for (i = 0; i <= k->n; i++) {
	 printf("[%d][%2d]%d\t", i, c, dyntbl[i][c]);
	 }
	 printf("\n");
	 }
	 }*/
}

void knapsack_solve_heuristic(knapsack_t *k, knapsack_heuristic_t h) {
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

	printf("%d %d %d ", k->id, k->n, k->solution.cost);
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

	knapsack_dynamic_t dynamic = UNKNOWN_DYNAMIC;
	double relative;
	int lines = 0;

	memset(&k, 0, sizeof(knapsack_t));

	while ((c = getopt(argc, argv, "b:d:fh:r:tp")) != -1) {
		switch (c) {
		case 'b':
			method = BRUTEFORCE;
			bruteforce_cut = atoi(optarg);
			break;
		case 'd':
			method = DYNAMIC;
			dynamic = atoi(optarg);
			break;
		case 'f':
			method = FPTAS;
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
			case DYNAMIC:
				switch (dynamic) {
				case UP:
					knapsack_solve_dynamic_up(&k);
					break;
				case DOWN:
					knapsack_solve_dynamic_down(&k);
					break;
				case UNKNOWN_DYNAMIC:
					assert(0);
					break;
				default:
					assert(0);
					break;
				}
				break;
			case FPTAS:
				knapsack_fptas(&k);
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

		relative = ((double) (k.solution.cost_best - k.solution.cost))
				/ (double) (k.solution.cost_best);

		if (relative > result.error_maximal) {
			result.error_maximal = relative;

			/*if (relative > 0) {
			 fprintf(stderr, "err on line: %d\n", lines);
			 }*/
		}

		result.error_relative += relative;

		if (!time_show) {
			knapsack_print(&k);
		}
	}

	result.time = omp_get_wtime() - result.time;

	result.error_relative = result.error_relative / (double) lines;
	printf("%lf_%lf_%lf\n", result.error_relative, result.error_maximal,
			result.time);

	if (time_show) {
		time_end = omp_get_wtime();
		printf("%lf\n", (time_end - time_start) / (double) repeat);
	}

	if (0)
		fprintf(stderr, "lower cuts=%d, upper cuts=%d\n", lc, uc);

	return (EXIT_SUCCESS);
}
