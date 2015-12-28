/*
 * Tomas Nesrovnal
 * nesro@nesro.cz
 * https://github.com/nesro/nesrotom-paa-2015
 */

/******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
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

#define MAX_ITEMS 500
#define MAX_CAPACITY 500000
#define MAX_COST 500000

/******************************************************************************/

inline void swap(int *a, int *b) {
	int *tmp;
	tmp = a;
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
	/* TODO: add pointer to knapsack */
	int items[MAX_ITEMS];
	int weight;
	int cost;
	int cost_best; /* obsolete */
	int fitness;
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
	UNKNOWN_METHOD, BRUTEFORCE, HEURISTIC, DYNAMIC, FPTAS, GA
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
	knapsack_solution_t s = { { 0 }, 0, 0, 0, 0 };
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

int malloc_2d(int ***arg_arr, int **arg_arr_data, int x, int y) {
	int **arr = *arg_arr;
	int *arr_data = *arg_arr_data;
	int i;

	arr = malloc(x * sizeof(int *));
	if (arr == NULL) {
		return (0);
	}

	arr_data = malloc(x * y * sizeof(int));
	if (arr_data == NULL) {
		return (0);
	}

	for (i = 0; i < x; i++) {
		arr[i] = arr_data + (i * y);
	}

	*arg_arr = arr;
	*arg_arr_data = arr_data;

	return (1);
}

void knapsack_solve_dynamic_up(knapsack_t *k) {
	int i;
	int w;

//	int dyntbl[MAX_ITEMS + 1][MAX_CAPACITY + 1];
	int **dyntbl = NULL;
	int *dyntbl_data = NULL;

	malloc_2d(&dyntbl, &dyntbl_data, k->n + 1, k->cap + 1);

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

	free(dyntbl);
	free(dyntbl_data);
}

void knapsack_fptas(knapsack_t *k, int fptas_eps) {
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
		cost_max = max(cost_max, k->items[i].cost);
	}

	double eps = ((double) fptas_eps) / 100.0;
	int scale = log((eps * cost_max) / k->n) / log(2);
	scale = max(0, scale);

	for (i = 0; i < k->n; i++) {
		k->items[i].cost >>= scale;
	}

	for (i = 0; i < k->n; i++) {
		cost_sum += k->items[i].cost;
	}

	assert(cost_sum < MAX_COST);

	for (i = 0; i <= k->n; i++) {
		for (c = 0; c <= cost_sum; c++) {
			dyntbl[i][c] = 0;
		}
	}

	for (i = 0; i <= cost_sum; i++) {
		dyntbl[0][i] = INT_MAX / 2;
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
			k->solution.cost <<= scale;
			break;
		}
	}
	for (i = 0; i < k->n; i++) {
		k->items[i].cost <<= scale;
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
/* GA */

/*  global variables */
int ga_population_g; /* size of population (how many genomes) */
int ga_in_tournament_g; /* how many genoms in tournament */
int ga_repeat_g; /* how many repeation cycles */
int ga_mutation_g; /* mutation rate, in 0.1 % */
int ga_cross_cnt_g;

void ga_cross_2p(knapsack_t *k, knapsack_solution_t *sa, knapsack_solution_t *sb) {
	int cross_point1 = rand() % k->n;
	int cross_point2 = rand() % k->n;
	int i;

	if (cross_point1 > cross_point2) {
		swap(&cross_point1, &cross_point2);
	}

	for (i = 0; i < k->n; i++) {
		if (i < cross_point1 && i > cross_point2) {
			swap(&sa->items[i], &sb->items[i]);
		}
	}
}

void ga_cross_1p(knapsack_t *k, knapsack_solution_t *sa, knapsack_solution_t *sb) {
	int cross_point1 = rand() % k->n;
	int i;

	for (i = 0; i < k->n; i++) {
		if (i < cross_point1) {
			swap(&sa->items[i], &sb->items[i]);
		}
	}
}

int ga_fitness(knapsack_t *k, knapsack_solution_t *s) {
	int i;
	int solution_cost = 0;
	int solution_weight = 0;

	for (i = 0; i < k->n; i++) {
		if (s->items[i] == 1) {
			solution_cost += k->items[i].cost;
			solution_weight += k->items[i].weight;
		}
	}

	if (solution_weight > k->cap) {
		return (0);
	}

	return (solution_cost);
}

void ga_mutate(knapsack_t *k, knapsack_solution_t *s) {
	int i;

	for (i = 0; i < k->n; i++) {
		if (rand() % 1000 <= ga_mutation_g) {
			s->items[i] = s->items[i] ? 0 : 1;
		}
	}
}

void ga_randomize(knapsack_t *k, knapsack_solution_t *s) {
	int i;

	for (i = 0; i < k->n; i++) {
		if ((rand() % 2) == 0) {
			s->items[i] = 0;
		} else {
			s->items[i] = 1;
		}
	}
}

int ga_fitness_cmp(const void *a, const void *b) {
	knapsack_solution_t *sa = (knapsack_solution_t *) a;
	knapsack_solution_t *sb = (knapsack_solution_t *) b;

	return (sb->fitness - sa->fitness);
}

/* sa = solution array */
knapsack_solution_t* ga_select(knapsack_t *k, knapsack_solution_t *sa) {
	knapsack_solution_t *selected[ga_in_tournament_g];
	int i;
	int best = 0;

	for (i = 0; i < ga_in_tournament_g; i++) {
		selected[i] = &sa[rand() % k->n];
	}

	for (i = 1; i < ga_in_tournament_g; i++) {
		if (selected[i]->fitness > selected[best]->fitness) {
			best = i;
		}
	}

	return (selected[best]);
}

/*
 https://edux.fit.cvut.cz/courses/BI-ZUM/_media/lectures/05-evolution-v3.0.pdf
 */
void ga_main(knapsack_t *k) {
	int i;
	int j;

	int popul;
	int in_tournament;
	int mutation;

	int best_popul;
	int best_in_tournament;
	int best_mutation;
	int best_fitness = -1;

	knapsack_solution_t *sa;
	knapsack_solution_t *sb;
	knapsack_solution_t *stmp;

	ga_repeat_g = 1000;

//./knapgen -I 42 -n 100 -N 1 -m 0.5 -k 0.5 -W 1000 -C 1000 -d 0 2>/dev/null >../knapsack/tests/knap_ga_100.inst.dat

	for (popul = 1000; popul <= 1000; popul += 50) {
		ga_population_g = popul;

		for (in_tournament = 50; in_tournament <= 50; in_tournament += 10) {
			ga_in_tournament_g = in_tournament;

			for (mutation = 8; mutation <= 8; mutation++) {
				ga_mutation_g = mutation;

				srand(0);

				sa = malloc(popul * sizeof(knapsack_solution_t));
				sb = malloc(popul * sizeof(knapsack_solution_t));
				assert(sa && sb);

				for (i = 0; i < popul; i++) {
					do {
						ga_randomize(k, &sa[i]);
						sa[i].fitness = ga_fitness(k, &sa[i]);
					} while (sa[i].fitness == 0);
				}

				for (j = 0; j < ga_repeat_g; j++) {
					for (i = 0; i < popul; i += 2) {
						memcpy(&sb[i + 0], ga_select(k, sa),
								sizeof(knapsack_solution_t));
						memcpy(&sb[i + 1], ga_select(k, sa),
								sizeof(knapsack_solution_t));

						ga_cross_2p(k, &sb[i + 0], &sb[i + 1]);
						//ga_cross_1p(k, &sb[i + 0], &sb[i + 1]);

						ga_mutate(k, &sb[i + 0]);
						ga_mutate(k, &sb[i + 1]);

						sb[i + 0].fitness = ga_fitness(k, &sb[i + 0]);
						sb[i + 1].fitness = ga_fitness(k, &sb[i + 1]);
					}

					stmp = sa;
					sa = sb;
					sb = stmp;
				}

				qsort(sa, popul, sizeof(knapsack_solution_t), ga_fitness_cmp);

				k->cost_best = sa[0].fitness;

				if (best_fitness < sa[0].fitness) {
					best_fitness = sa[0].fitness;
					best_popul = popul;
					best_in_tournament = in_tournament;
					best_mutation = mutation;
				}

				printf("%d %d %d %d\n", sa[0].fitness, popul, in_tournament,
						mutation);

				free(sa);
				free(sb);
			}
		}
	}

	printf("best_fitness=%d best_popul=%d best_in_tour=%d best_mutati=%d\n",
			best_fitness, best_popul, best_in_tournament, best_mutation);
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
	int fptas_eps = -1;
	int show_me_error_and_time = 0;

	knapsack_dynamic_t dynamic = UNKNOWN_DYNAMIC;
	double relative;
	int lines = 0;

	//srand (time(NULL));
	srand(0);

	memset(&k, 0, sizeof(knapsack_t));

	while ((c = getopt(argc, argv, "b:ed:f:Gh:r:tp")) != -1) {
		switch (c) {
		case 'b':
			method = BRUTEFORCE;
			bruteforce_cut = atoi(optarg);
			break;
		case 'e':
			show_me_error_and_time = 1;
			break;
		case 'd':
			method = DYNAMIC;
			dynamic = atoi(optarg);
			break;
		case 'G':
			method = GA;
			break;
		case 'f':
			method = FPTAS;
			fptas_eps = atoi(optarg);
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
				knapsack_fptas(&k, fptas_eps);
				break;
			case HEURISTIC:
				knapsack_solve_heuristic(&k, heuristic);
				break;
			case GA:
				ga_main(&k);
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

	if (result.error_relative < 0) {
		result.error_relative = 0;
	}

	if (show_me_error_and_time) {
		printf("%lf_%lf_%lf\n", result.error_relative, result.error_maximal,
				result.time);
	}

	if (time_show) {
		time_end = omp_get_wtime();
		printf("%lf\n", (time_end - time_start) / (double) repeat);
	}

	if (0)
		fprintf(stderr, "lower cuts=%d, upper cuts=%d\n", lc, uc);

	return (EXIT_SUCCESS);
}
