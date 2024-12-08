#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/pthread.h"

#define N 10
pthread_mutex_t mutex;

void *foo()
{
  pthread_mutex_lock(&mutex);
  printf("%d: work\n", gettid());
  pthread_mutex_unlock(&mutex);

  return 0;
}

int main(int argc, char const *argv[])
{
  pthread_t p[N];
  int rc;

  pthread_mutex_init(&mutex);
  for(int i = 0; i < N; i++){
    rc = pthread_create(&p[i], foo, 0, 0);
    if(rc == 0)
      printf("panic, i = %d\n", rc);
  }
  for(int i = 0; i < N; i++)
    pthread_join(p[i], 0);
  
  return 0;
}
