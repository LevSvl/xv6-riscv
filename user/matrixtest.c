#include "kernel/types.h"
#include "user/user.h"


/*   blockingcalltest's params  */

#define fail(s) printf("FAILED: %s\n", s);

#define CYCLES 1 // number of repeats matrices multiplication

// matrices sizes
#define N 12
#define M 500000

int A[N][M], B[N][M] , C[N][M], D[N][M]; //E[N][M]; // matrices for test
int tmsleep = 1; // time that thread sleeps after each row of matrix

void init_matrix(int m[N][M]);


/*  User-threads  */

extern void thread_switch( uint64,  uint64);

/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  4


struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct thread {
  char       stack[STACK_SIZE]; /* the thread's stack */
  int        state;             /* FREE, RUNNING, RUNNABLE */
  struct context context;

};
struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch( uint64,  uint64);
              
void 
thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule(). It needs a stack so that the first thread_switch() can
  // save thread 0's state.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

void 
thread_schedule(void)
{
  struct thread *t, *next_thread;

  /* Find another runnable thread. */
  next_thread = 0;
  t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD)
      t = all_thread;
    if(t->state == RUNNABLE) {
      next_thread = t;
      break;
    }
    t = t + 1;
  }

  if (next_thread == 0) {
    // if all child-threads are done, then
    // thread scheduler returns control to main-thread 

    // printf("thread_schedule: no runnable threads\n");
    current_thread = &all_thread[0];
    current_thread->state = RUNNING;
    thread_switch((uint64)&t->context, (uint64)&current_thread->context);
    exit(0);  
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    thread_switch((uint64)&t->context, (uint64)&current_thread->context);  
  } 
  else
    next_thread = 0;
}

void 
thread_create(void (*func)())
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;
  t->context.ra = (uint64)func;
  t->context.sp = (uint64)t->stack+STACK_SIZE;
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  thread_schedule();
}

volatile int a_started, b_started, c_started;
volatile int a_n, b_n, c_n;

void 
thread_a(void)
{
  a_started = 1;
  while(b_started == 0 || c_started == 0)
    thread_yield();
  
  for(int k = 0; k < CYCLES; k++){
    for(int i = 0; i < N/3; i++){
      for(int j = 0; j < M; j++)
        D[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_b(void)
{
  b_started = 1;
  while(a_started == 0 || c_started == 0)
    thread_yield();

  for(int k = 0; k < CYCLES; k++){
    for(int i = N/3; i < 2*(N/3); i++){
      for(int j = 0; j < M; j++)
        D[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_c(void)
{
  c_started = 1;
  while(a_started == 0 || b_started == 0)
    thread_yield();
  
  for(int k = 0; k < CYCLES; k++){
    for(int i = 2*(N/3); i < 3*(N/3); i++){
      for(int j = 0; j < M; j++)
        D[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }

  current_thread->state = FREE;
  thread_schedule();
}

// blockingcalltest compares the time taken to multiply
// two matrices in a single user-thread with the time it takes
// to accomplish this with multiple user-threads. 
// The user-threads multiply different rows of the matrices,
// so no race conditions arise. 
// There are three user-threads in total, each responsible for
// multiplying one-third of the number of rows in the matrix.
// Therefore, for the program to function correctly, the value
// of N should be divisible by 3
int
main(int argc, char const *argv[])
{
  int tmstart1, tmstart3, tmstart3k;

  init_matrix(A);
  init_matrix(B);
  init_matrix(C);
  init_matrix(D);
  // init_matrix(E);

  /*  1 user-thread...   */

  tmstart1 = uptime();

  for(int k = 0; k < CYCLES; k++){
    for(int i = 0; i < N; i++){
      for(int j = 0; j < M; j++)
        C[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }

  printf("1 user-thread mean time: %d\n", uptime() - tmstart1);

  /*  3 user-threads...  */
  
  a_started = b_started = c_started = 0;
  a_n = b_n = c_n = 0;
  thread_init();

  tmstart3 = uptime();

  thread_create(thread_a);
  thread_create(thread_b);
  thread_create(thread_c);

  current_thread->state = FREE;
  
  // give control to thread-scheduler
  // it'll save main-thread's context and 
  // give control to created child-threads
  thread_schedule();

  printf("3 user-threads mean time: %d\n", uptime() - tmstart3);

  for(int i = 0; i < N; i++)
    for(int j = 0; j < M; j++)
      if(C[i][j] != D[i][j]){
        fail("matrices are different");
        exit(1);
      }

  /*  3 kernel-threads...  */
  extern void* kthread_a(), *kthread_b(), *kthread_c();

  void *stacks = malloc(4096*3);

  tmstart3k = uptime();

  clone(kthread_a, 0, 0, stacks);
  clone(kthread_b, 0, 0, stacks + 4096);
  clone(kthread_c, 0, 0, stacks + 4096*2);

  join(stacks, 0);
  join(stacks + 4096, 0);
  join(stacks + 4096*2, 0);

  printf("3 kernel-threads mean time: %d\n", uptime() - tmstart3k);


  printf("blockingcalltest is OK\n");

  // there is no wrapper-function _main
  // for blockingcalltest so main must
  // use exit() insted of return
  exit(0);
}

void
init_matrix(int m[N][M])
{
  for(int i = 0; i < N; i++)
    for(int j = 0; j < M; j++)
      m[i][j] = (i + j) / 2;
}


// kernel-threads
void* 
kthread_a(void)
{  
  for(int k = 0; k < CYCLES; k++){
    for(int i = 0; i < N/3; i++){
      for(int j = 0; j < M; j++)
        D[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }
  return 0;
}

void*
kthread_b(void)
{
  for(int k = 0; k < CYCLES; k++){
    for(int i = N/3; i < 2*(N/3); i++){
      for(int j = 0; j < M; j++)
        D[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }
  return 0;
}

void* 
kthread_c(void)
{  
  for(int k = 0; k < CYCLES; k++){
    for(int i = 2*(N/3); i < 3*(N/3); i++){
      for(int j = 0; j < M; j++)
        D[i][j] = A[i][j] * B[i][j];
      sleep(tmsleep);
    }
  }
  return 0;
}
