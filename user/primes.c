/*
 * The sieve of Eratosthenes simulation with pipes
 * This idea is due to Doug McIlroy, inventor of Unix pipes
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
pipeline(int pipe_left[2])
{
    int pipe_right[2] = {-1, -1};
    int div, num, pid = -1;

    read(pipe_left[0], &div, 1);
    printf("prime %d\n", div);

    /* close file descriptors that a process doesn't need */
    close(pipe_left[1]);
    close(0);

    /* redirect input to read numbers from left node */
    dup(pipe_left[0]);

    while (read(pipe_left[0], &num, 1) != 0) {
        if ((num % div) != 0) {
            if (pipe_right[0] < 0) {
                /* ok, now we need to create next node in pipeline */
                
                /* create next pipe */
                pipe(pipe_right);

                pid = fork();
        
                if (pid < 0) {
                    exit(1);
                } else if (pid == 0) {
                    /* continue pipeline */
                    pipeline(pipe_right);
                    exit(0);
                }

                /* dont need to read as here we are source only */
                close(pipe_right[0]);
            }

            write(pipe_right[1], &num, sizeof(int));
        }
    }

    // close pipeline so next node will read 0
    close(pipe_right[1]);

    // wait for the next node exit
    if(pid > 0)
        wait((int *)0);
}

int
main(int argc, char *argv[])
{
    int p[2], num, pid;


    /* create first pipe */
    pipe(p);

    pid = fork();
    
    if (pid < 0) {
        exit(1);
    } else if (pid == 0) {
        /* start pipeline */
        pipeline(p);
        exit(0);
    }

    /* close unusable file descriptors and start seeding */
    close(p[0]);
    close(0);
    close(1);
    close(2);

    for(num = 2; num < 36; num++) {
        write(p[1], &num, sizeof(int));
    }

    /* stop seeding and wait until the entire pipeline terminates */
    close(p[1]);
    wait((int *)0);

    exit(0);
}
