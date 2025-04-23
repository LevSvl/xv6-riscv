#include "kernel/types.h"
#include "user/user.h"
#include "user/testslib.h"
#include "user/pthread.h"

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int fill_array_with_randint(int *array, int len)
{
    for (int i = 0; i < len; i += 1)
    {
        int x = uptime();
        x ^= (x << i);
        x ^= (x >> 17);
        x ^= (x << 5);
        array[i] = x % 100;
    }

    return OK;
}

void print_array(int *array, int len)
{
    for (int i = 0; i < len; i++)
    {
        fprintf(1, "%d ", array[i]);
    }
    fprintf(1, "\n");
}

/* Prepare arrays */
int prepare_arrays_randint(int **a1, int **a2, int **a4, int len)
{
    *a1 = (int *)malloc(sizeof(int) * len);
    *a2 = (int *)malloc(sizeof(int) * len);
    *a4 = (int *)malloc(sizeof(int) * len);

    if (*a1 == 0 || *a2 == 0 || *a4 == 0)
    {
        return FAILED;
    }

    if (fill_array_with_randint(*a1, len) == FAILED)
    {
        return FAILED;
    }

    memcpy(*a2, *a1, sizeof(int) * len);
    memcpy(*a4, *a1, sizeof(int) * len);

    if ((memcmp(*a2, *a1, sizeof(int) * len) != OK) || (memcmp(*a4, *a1, sizeof(int) * len) != OK))
    {
        return FAILED;
    }

    return OK;
}

void *swap_multi_thread(void *xarray, void *xidx)
{
    int *array = (int *)xarray;
    int idx = (long long int)xidx;

    if (array[idx] > array[idx + 1])
    {
        swap(&array[idx], &array[idx + 1]);
    }

    return (void *)OK;
}

void print_header()
{
    fprintf(1, "                                     TESTING MULTITHREADING (XV6)                             \n");
    fprintf(1, "                                                                                              \n");
    fprintf(1, "______________________________________________________________________________________________\n");
    fprintf(1, "|           TEST             |                    RESULT TIME IN NANOSECONDS                  | \n");
    fprintf(1, "                                                                                              \n");
    fprintf(1, "                                  1 THREAD            2 THREADS            4 THREADS          \n");
}

void print_results(test_t *test)
{
    if (test->status != OK)
    {
        fprintf(1, "| %s                          |                            FAILED                               |\n");
    }
    else
    {

        fprintf(1, "  %s                                ", test->name);
        fprintf(1, "%l                  ", test->result_in_nanoseconds[0]);
        fprintf(1, "%l                  ", test->result_in_nanoseconds[1]);
        fprintf(1, "%l                  \n", test->result_in_nanoseconds[2]);
    }

    fprintf(1, "                                                                                              \n");
}
