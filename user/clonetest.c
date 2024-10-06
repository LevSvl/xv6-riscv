#include "kernel/types.h"
#include "user/user.h"
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

int
main(int argc, char const *argv[])
{ 
  printf("start multithreading\n");
  
  threadExecutingTest();
  sharingDataTest();
  simpleJoinTest();
  threadReturnTest();
  childExitTest();

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
  void *stack = (void*)malloc(STACK_SIZE);
  clone(foo, 0, 0, stack);
  join(stack, 0);
  free(stack);
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
  void *stack = (void*)malloc(STACK_SIZE);
  clone(changeData, 0, 0, stack);
  join(stack, 0);
  free(stack);

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
  void *stack = (void*)malloc(STACK_SIZE*2);
  clone(veryLongFunc, 0, 0, stack);
  join(stack, 0);
  // exit(0);
  return 0;
}

void
childExitTest()
{
  void *stack = (void*)malloc(STACK_SIZE);
  clone(parentFunc, 0, 0, stack);
  // sleep(10);
  // free(stack);
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
  void *stack = (void*)malloc(STACK_SIZE);
  int rc = clone(func_a, 0, 0, stack);
  
  if(rc < 0){
    printf("simpleJoinTest FAILED: thread inallocable\n");
    return;
  }

  join(stack, 0);

  free(stack);

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

  void *stack = malloc(4096); 
  
  clone(power, (void*)a, 0, stack);
  join(stack, &r);

  if(r == a)
    printf("threadReturnTest is OK\n");
  else
    printf("threadReturnTest is FAILED\n");

  free(stack);
}
