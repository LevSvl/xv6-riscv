/****************** DEFINITIONS AND DECLARATIONS **********************/
#ifndef TESTLIB_H
#define TESTLIB_H

typedef unsigned long long clock_t;

typedef struct test_t {
    char name[28];
    int (*func)(struct test_t *);
    int status;
    clock_t result_in_ticks[3];
    clock_t result_in_nanoseconds[3];
} test_t;
extern test_t tests[];
enum { OK, FAILED };

void print_header();
void print_results(test_t *test);

void swap(int *a, int *b);
void *swap_multi_thread(void *xarray, void *xidx);

int fill_array_with_randint(int *, int);
void print_array(int *array, int len);
int prepare_arrays_randint(int **a1, int **a2, int **a4, int len);


#endif // TESTLIB_H
