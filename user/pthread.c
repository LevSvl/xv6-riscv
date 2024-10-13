#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/pthread.h"

#define DEFAULT_STACK_SIZE  4096

int
pthread_create(pthread_t *thread_stack, void *(*start_routine)(void *, void *),\
                  void *arg1, void *arg2)
{
  // allocate memory for stack
  *thread_stack = (pthread_t)malloc(DEFAULT_STACK_SIZE);
  if(thread_stack == 0)
    return -1;
  
  return clone(start_routine, arg1, arg2, (void*)*thread_stack);
}

int
pthread_join(pthread_t thread_stack, void *retval)
{
  int tid;
  
  tid = join((void*)thread_stack, retval);

  free((void*)thread_stack);

  return tid;
}

int
pthread_cancel(pthread_t thread)
{
  return cancel((void*)thread);
}
