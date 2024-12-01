#include "kernel/types.h"
#include "user/user.h"
#include "user/pthread.h"
#include "kernel/fcntl.h"


#define STACK_SIZE  4096

// check that child thread created
// with clone succsessfully executes
extern void threadExecutingTest();

// check that main and child threads 
// all have right .data values that  
// can be accessed by every thread
extern void sharingDataTest();

// check that exit() called by
// child thread will not be a
// problem for other threads execution
extern void childExitTest();

// check that parent thread waits
// for the end of the child execution
// and then gets the status of child completion
extern void simpleJoinTest();

// check that child threads always
// have correct value of Thread ID
// TODO: extern void allocTidTest();

// attributes test
extern void threadReturnTest(); 

extern void usingMutexTest();

extern void allocStacksTest();

int
main(int argc, char const *argv[])
{ 
  printf("start multithreading\n");
  
  // threadExecutingTest();
  // sharingDataTest();
  // simpleJoinTest();
  // threadReturnTest();
  usingMutexTest();

  printf("end multithreading\n");
  exit(0);
}

void* 
foo()
{
  printf("threadExecutingTest is OK\n");
  return 0;
}

void
threadExecutingTest()
{
  pthread_t p;
  pthread_create(&p, foo, 0, 0);
  pthread_join(p, 0);
}


int data[] = {1, 2, 3, 4, 5};

void*
changeData()
{
  printf("changing data\n");
  for(int i = 0; i < (sizeof(data)/ sizeof(data[0])); i++){
    if(data[i] != i + 1){
      printf("sharingDataTest FAILED: invalid data\n");
      break;
    }
    data[i] -= 1;
  }
  return 0;
}

void
sharingDataTest()
{
  pthread_t p;
  pthread_create(&p, changeData, 0, 0);
  pthread_join(p, 0);

  for(int i = 0; i < (sizeof(data)/ sizeof(data[0])); i++){
    if(data[i] != i){
      printf("sharingDataTest FAILED: data is not changed\n");
      return;
    }
  }

  printf("sharingDataTest OK\n");
}

void*
veryLongFunc()
{
  // sleep(100);
  return 0;
}

void*
parentFunc()
{
  pthread_t pp;
  pthread_create(&pp, veryLongFunc, 0, 0);
  pthread_join(pp, 0);
  // exit(0);
  return 0;
}

void
childExitTest()
{
  pthread_t p;
  pthread_create(&p, parentFunc, 0, 0);
  printf("childExitTest FAILED\n");
}

int sum = 0;

void*
func_a()
{
  sum = 1;
  for(int i = 0; i < 10; i++){
    sum += i;
  }
  sleep(1);
  sum -= 45;

  return 0;
}

void
simpleJoinTest()
{
  pthread_t p;
  int rc;

  rc = pthread_create(&p, func_a, 0, 0);
  
  if(rc < 0){
    printf("simpleJoinTest FAILED: thread inallocable\n");
    return;
  }

  pthread_join(p, 0);

  if(sum != 1){
    printf("simpleJoinTest FAILED\n");
    return;
  }
  printf("simpleJoinTest OK\n");
}

void*
power(void *a, void *b)
{
  long long c = (long long)a;
  return (void*)c;
}

void
threadReturnTest()
{
  long long a = 6;
  long long r;
  pthread_t p;
  
  pthread_create(&p, power, (void*)a, 0);
  pthread_join(p, &r);

  if(r == a)
    printf("threadReturnTest is OK\n");
  else
    printf("threadReturnTest is FAILED\n");
}

uint64 n = 100;
int num_threads = 3;
int repeats = 10000;
pthread_mutex_t mutex;

void
usingMutexTest()
{
  int *arr = (int*)malloc(n*sizeof(int));
  pthread_t *tasks = (pthread_t*)malloc(num_threads*sizeof(pthread_t));

  memset(arr, 0, n*sizeof(int));
  void *inrement_arr(void *arr, void* sz);

  pthread_mutex_init(&mutex);

  for(int i = 0; i < num_threads; i++){
    pthread_create(&tasks[i], inrement_arr, (void*)arr, (void*)n);
  }

  for(int i = 0; i < num_threads; i++){
    pthread_join(tasks[i], 0);
  }

  for(int i = 0; i < n; i++)
    printf("%d:  %d\n", i, arr[i]);

  free(arr);
  free(tasks);
}

void*
inrement_arr(void * xarr, void* xsz)
{
  int *arr = (int*)xarr;
  uint64 sz = (uint64)xsz;

  for(int i = 0; i < repeats; ++i)
    for(int j = 0; j < sz; j++){
      pthread_mutex_lock(&mutex);
      arr[j] += 1;
      pthread_mutex_unlock(&mutex);
    }
  return 0;
}
