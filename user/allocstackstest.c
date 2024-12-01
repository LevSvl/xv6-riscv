#include "kernel/types.h"
#include "user/user.h"
#include "user/pthread.h"
#include "kernel/fcntl.h"

#define N 12

void *nop(){  
  while(1)
    ;
  return 0;
}

void
maxThreads()
{
  pthread_t tasks[5];
  for(int i = 0; i < 5; i++){
    int tid = pthread_create(tasks + i, nop, 0, 0);
    printf("pid: %d: tid: %d\n", getpid(), tid);
  }

  for(int i = 0; i < 5; i++){
    int tid = join(tasks + i, 0);
    printf("pid: %d: tid: %d\n", getpid(), tid);
  }
}

void
allocStacksTest()
{
  int pid1, pid2, pid3, pid4;
  int tid;

  if((pid1 = fork()) < 0){
    printf("fork 1: failed");
    exit(1);
  }

  if(pid1 == 0){
    pthread_t tasks1[N/4];
    for(int i = 0; i < N/4; i++){
      tid = pthread_create(tasks1 + i, nop, 0, 0);
      printf("pid: %d: tid: %d\n", getpid(), tid);
    }
    wait(0);
    exit(0);
  }

  if((pid2 = fork()) < 0){
    printf("fork 2: failed");
    exit(1);
  }

  sleep(5);
  if(pid2 == 0){
    pthread_t tasks2[N/4];
    for(int i = 0; i < N/4; i++){
      tid = pthread_create(tasks2 + i, nop, 0, 0);
      printf("pid: %d: tid: %d\n", getpid(), tid);
    }
    wait(0);
    exit(0);
  }

  if((pid3 = fork()) < 0){
    printf("fork 3: failed");
    exit(1);
  }

  sleep(5);
  if(pid3 == 0){
    pthread_t tasks3[N/4];
    for(int i = 0; i < N/4; i++){
      tid = pthread_create(tasks3 + i, nop, 0, 0);
      printf("pid: %d: tid: %d\n", getpid(), tid);
    }
    wait(0);
    exit(0);
  }

  sleep(5);
  if((pid4 = fork()) < 0){
    printf("fork 4: failed");
    exit(1);
  }

  if(pid4 == 0){
    pthread_t tasks4[N/4];
    for(int i = 0; i < N/4; i++){
      tid = pthread_create(tasks4 + i, nop, 0, 0);
      printf("pid: %d: tid: %d\n", getpid(), tid);
    }
    wait(0);
    exit(0);
  }

  sleep(10);

  pthread_t last_task;
  tid = pthread_create(&last_task, nop, 0, 0); 
  printf("pid: %d: tid: %d\n", getpid(), tid);

  pthread_t last_task1;
  tid = pthread_create(&last_task1, nop, 0, 0); 
  printf("pid: %d: tid: %d\n", getpid(), tid);
}

void
allocStacksTest1()
{
  int pid1;
  int tid;

  for(int i = 0; i < 40; i++ ){
    if((pid1 = fork()) < 0){
      // printf("fork %d: failed\n", i);
      exit(1);
    }

    if(pid1 == 0){
      pthread_t tasks1[4];
      for(int i = 0; i < N/4; i++){
        tid = pthread_create(tasks1 + i, nop, 0, 0);
        printf("pid: %d: tid: %d\n", getpid(), tid);
      }
      wait(0);
      exit(0);
    }
  }

  sleep(10);

  pthread_t last_task;
  tid = pthread_create(&last_task, nop, 0, 0); 
  printf("pid: %d: tid: %d\n", getpid(), tid);

  pthread_t last_task1;
  tid = pthread_create(&last_task1, nop, 0, 0); 
  printf("pid: %d: tid: %d\n", getpid(), tid);
}

int main()
{
  // maxThreads();
  // allocStacksTest();
  // allocStacksTest1();
  return 0;
}