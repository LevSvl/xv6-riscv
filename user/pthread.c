#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/pthread.h"


/*
  #TODO:
  - thread funcs without return, without nessesary 2 args
  - tlibtest last test
  - sleeping locks
  - cond locks
  - gettid() syscall
  - signals between threads when process grows or seeks
  - rise NTHREAD variable
  - cpu masks
  
  - compare speed, memory, stability with ususal Xv6 thread model
*/


/* Thread manipulations */

#define DEFAULT_STACK_SIZE  4096
#define MAXIMUM_ACTIVE_THREADS 12*4
#define ALL_MEM_FOR_STACKS  (DEFAULT_STACK_SIZE*MAXIMUM_ACTIVE_THREADS)

typedef pthread_mutex_t __stacks_lock_t;

typedef struct __stack{
  uint64 stackbase;
  int is_used;
  __stacks_lock_t s_lock;
} __stack_t;

#define STACK_ALIGNED __attribute__((aligned(DEFAULT_STACK_SIZE)))
static uint8 STACK_ALIGNED all_stacks_mem[ALL_MEM_FOR_STACKS];

static uint8 __current_threads_system_cnt = 0;

static __stack_t stacks_desc[MAXIMUM_ACTIVE_THREADS];
__stacks_lock_t stacks_lock = {0}; // stacks_lock->flag = 0
static int initial = 1;


#define init(s)   (pthread_mutex_init(s))
#define lock(s)   (pthread_mutex_lock(s))
#define unlock(s) (pthread_mutex_unlock(s))

// init allocated stacks once, 
// get free stack from all_thread_stacks 
uint64
__alloc_thread_stack()
{
  __stack_t *s;
  uint64 dst_stack_addr;
  int i;

  /* initial code */
  if(initial){
    lock(&stacks_lock);
    initial = 0;
    
    for(i = 0; i < MAXIMUM_ACTIVE_THREADS; i++){
      s = &stacks_desc[i];
      s->is_used = 0;
      s->stackbase = (uint64)&all_stacks_mem[i * DEFAULT_STACK_SIZE];
      init(&s->s_lock);
    }

    __sync_synchronize();
    unlock(&stacks_lock);
  }
  

  /* regular code */
  int found = 0;

  for(i = 0; i < MAXIMUM_ACTIVE_THREADS; i++){
    s = &stacks_desc[i];
    
    lock(&s->s_lock);

    if(!s->is_used){
      dst_stack_addr = s->stackbase;
      s->is_used = 1;
      found = 1;
      unlock(&s->s_lock);
      break;
    }
    unlock(&s->s_lock);
  }

  if(!found)
    return 0;

  lock(&stacks_lock);

  if(__current_threads_system_cnt == MAXIMUM_ACTIVE_THREADS){
    unlock(&stacks_lock);

    lock(&stacks_desc[i].s_lock);
    stacks_desc[i].is_used = 0;
    unlock(&stacks_desc[i].s_lock);
    
    return 0;
  }
  __current_threads_system_cnt++;

  unlock(&stacks_lock);

  return dst_stack_addr;
}

int
__dealloc_thread_stack(uint64 stack_addr)
{
  __stack_t *s;
  int found = 0;

  for(int i = 0; i < MAXIMUM_ACTIVE_THREADS; i++){
    s = &stacks_desc[i];

    lock(&s->s_lock);
    if(s->stackbase == stack_addr){
      found = 1;
      s->is_used = 0;
      unlock(&s->s_lock);
      break;
    }
    unlock(&s->s_lock); 
  }

  if(!found)
    return -1;

  lock(&stacks_lock);

  if(__current_threads_system_cnt == 0){
    unlock(&stacks_lock);
    return -1;
  }

  __current_threads_system_cnt -= 1;
  unlock(&stacks_lock);

  return 0;
};

int
pthread_create(pthread_t *thread_stack, void *(*start_routine)(void *, void *),\
                  void *arg1, void *arg2)
{
  // allocate memory for stack
  *thread_stack = (pthread_t)__alloc_thread_stack();
  if(thread_stack == 0)
    return -1;
  return clone(start_routine, arg1, arg2, (void*)*thread_stack);
}

int
pthread_join(pthread_t thread_stack, void *retval)
{
  int tid;
  
  tid = join((void*)thread_stack, retval);

  if(__dealloc_thread_stack((uint64)thread_stack) < 0)
    return -1;

  return tid;
}

int
pthread_cancel(pthread_t thread)
{
  return cancel((void*)thread);
}


/* Mutex manipulations */

void
pthread_mutex_init(pthread_mutex_t *mutex)
{
  mutex->locked = 0;
}

void
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  while(__sync_lock_test_and_set(&mutex->locked, 1) == 1)
    ;
  __sync_synchronize();
}

void
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  __sync_lock_release(&mutex->locked);
  __sync_synchronize();
}
