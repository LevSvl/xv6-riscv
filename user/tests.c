#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "user/user.h"
#include "user/pthread.h"
#include "kernel/fcntl.h"
#include "user/testslib.h"

/***************************** CONFIGS *****************************/
#define CONFIG_THREADS_1
#define CONFIG_THREADS_2 
#define CONFIG_THREADS_4
/***************************** TESTS *****************************/

/*
 *     Чет-нечетная сортировка, в одном потоке, в нескольких потоках.
 * Производительность замеряется в количестве тактов, необходимых для
 * выполнения вычислений.
 */
int even_odd_sort_test(test_t *test)
{
#if defined(CONFIG_THREADS_1) || defined(CONFIG_THREADS_2) || defined(CONFIG_THREADS_4)
    int *array;
#endif
    int *array_one_thread, *array_two_threads, *array_four_threads;
    int len_of_array = 500;

#if defined(CONFIG_THREADS_2) || defined(CONFIG_THREADS_4)
    pthread_t threads[4];
#endif
    int cycles_number = 1;
    clock_t clk_mean_one_thread = 0, clk_mean_two_threads = 0, clk_mean_four_threads = 0;

    for (int cycle = 1; cycle <= cycles_number; cycle++)
    {
        if (prepare_arrays_randint(&array_one_thread, &array_two_threads,
                                   &array_four_threads, len_of_array) == FAILED)
        {
            return FAILED;
        }

#ifndef CONFIG_THREADS_1
        free(array_one_thread);
#endif
#ifndef CONFIG_THREADS_2
        free(array_two_threads);
#endif
#ifndef CONFIG_THREADS_4
        free(array_four_threads);
#endif

#ifdef CONFIG_THREADS_1
        /* One thread sort start */
        array = array_one_thread;
        clock_t clk_start_one_thread = uptime();

        for (int i = 0; i < len_of_array - 1; i++)
        {
            for (int even_idx = 0; even_idx < len_of_array - 1; even_idx += 2)
            {
                if (array[even_idx] > array[even_idx + 1])
                {
                    swap(&array[even_idx], &array[even_idx + 1]);
                }
            }

            for (int odd_idx = 1; odd_idx < len_of_array - 1; odd_idx += 2)
            {
                if (array[odd_idx] > array[odd_idx + 1])
                {
                    swap(&array[odd_idx], &array[odd_idx + 1]);
                }
            }
        }

        clock_t clk_end_one_thread = uptime();

        /* check results */
        for (int j = 0; j < len_of_array - 1; j++)
        {
            if (array_one_thread[j] > array_one_thread[j + 1])
            {
                free(array_one_thread);
#ifdef CONFIG_THREADS_2
                free(array_two_threads);
#endif
#ifdef CONFIG_THREADS_4
                free(array_four_threads);
#endif
                return FAILED;
            }
        }
#endif // CONFIG_THREADS_1

#ifdef CONFIG_THREADS_2
        /* two threads sort start */
        array = array_two_threads;

        clock_t clk_start_two_threads = uptime();
        for (int i = 0; i < len_of_array - 1; i++)
        {
            for (long even_idx = 0; even_idx < len_of_array - 1; even_idx += 4)
            {
                pthread_create(&threads[0], swap_multi_thread, (void *)array, (void *)even_idx);
                swap_multi_thread((void *)array, (void *)(even_idx + 2));

                pthread_join(threads[0], 0);
            }

            for (long odd_idx = 1; odd_idx < len_of_array - 1; odd_idx += 4)
            {
                pthread_create(&threads[0], swap_multi_thread, (void *)array, (void *)odd_idx);
                swap_multi_thread((void *)array, (void *)(odd_idx + 2));

                pthread_join(threads[0], 0);
            }
        }
        clock_t clk_end_two_threads = uptime();

        /* check results */
        for (int j = 0; j < len_of_array - 1; j++)
        {
            if (array_two_threads[j] > array_two_threads[j + 1])
            {
                free(array_two_threads);
#ifdef CONFIG_THREADS_1
                free(array_one_thread);
#endif
#ifdef CONFIG_THREADS_4
                free(array_four_threads);
#endif
                return FAILED;
            }
        }
#endif // CONFIG_THREADS_2

#ifdef CONFIG_THREADS_4
        /* four threads sort start */
        array = array_four_threads;
        clock_t clk_start_four_threads = uptime();

        for (int i = 0; i < len_of_array - 1; i++)
        {
            for (long even_idx = 0; even_idx < len_of_array - 1; even_idx += 8)
            {
                pthread_create(&threads[0], swap_multi_thread, (void *)array, (void *)even_idx);
                pthread_create(&threads[1], swap_multi_thread, (void *)array, (void *)(even_idx + 2));
                pthread_create(&threads[2], swap_multi_thread, (void *)array, (void *)(even_idx + 4));
                swap_multi_thread((void *)array, (void *)(even_idx + 6));

                pthread_join(threads[0], 0);
                pthread_join(threads[1], 0);
                pthread_join(threads[2], 0);
            }

            for (long odd_idx = 1; odd_idx < len_of_array - 1; odd_idx += 8)
            {
                pthread_create(&threads[0], swap_multi_thread, (void *)array, (void *)odd_idx);
                pthread_create(&threads[1], swap_multi_thread, (void *)array, (void *)(odd_idx + 2));
                pthread_create(&threads[2], swap_multi_thread, (void *)array, (void *)(odd_idx + 4));
                swap_multi_thread((void *)array, (void *)(odd_idx + 6));

                pthread_join(threads[0], 0);
                pthread_join(threads[1], 0);
                pthread_join(threads[2], 0);
            }
        }
        clock_t clk_end_four_threads = uptime();

        /* check results */
        for (int j = 0; j < len_of_array - 1; j++)
        {
            if (array_four_threads[j] > array_four_threads[j + 1])
            {
                free(array_four_threads);
#ifdef CONFIG_THREADS_1
                free(array_one_thread);
#endif
#ifdef CONFIG_THREADS_2
                free(array_two_threads);
#endif
                return FAILED;
            }
        }
#endif // CONFIG_THREADS_4

        /* this iteraton done, calculating time, go next iteration */
#ifdef CONFIG_THREADS_1
        clk_mean_one_thread += (clk_end_one_thread - clk_start_one_thread);
        free(array_one_thread);
#endif
#ifdef CONFIG_THREADS_2
        clk_mean_two_threads += (clk_end_two_threads - clk_start_two_threads);
        free(array_two_threads);
#endif
#ifdef CONFIG_THREADS_4
        clk_mean_four_threads += (clk_end_four_threads - clk_start_four_threads);
        free(array_four_threads);
#endif

#if defined(CONFIG_THREADS_1) || defined(CONFIG_THREADS_2) || defined(CONFIG_THREADS_4)
        array = 0;
#endif
    }

    /* Count and output results */
#ifdef CONFIG_THREADS_1
    clk_mean_one_thread /= cycles_number;
#endif
#ifdef CONFIG_THREADS_2
    clk_mean_two_threads /= cycles_number;
#endif
#ifdef CONFIG_THREADS_4
    clk_mean_four_threads /= cycles_number;
#endif

    test->result_in_nanoseconds[0] = clk_mean_one_thread;
    test->result_in_nanoseconds[1] = clk_mean_two_threads;
    test->result_in_nanoseconds[2] = clk_mean_four_threads;

    return OK;
}

/*****************************************************************/
void drive_tests()
{
    struct test_t *test = &tests[0];

    while (test->func != 0)
    {
        int result_code = fork();

        if (result_code == 0)
        {
            test->status = test->func(test);
            print_results(test);
            exit(OK);
        }

        wait((int *)0);
        test++;
    }
}

int main(int argc, char const *argv[])
{
    print_header();
    drive_tests();
}

test_t tests[] = {
    {.name = "Sort", .func = even_odd_sort_test, .status = OK},
    {.func = 0},
};
