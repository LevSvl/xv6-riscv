#include "kernel/types.h"
#include "user/user.h"

#define NANO_SEC(t)  (t*100)
#define CYCLES    1000

typedef unsigned long long time_t;
typedef unsigned long long my_int_t;

my_int_t PGSIZE = 4096;
my_int_t NUMPAGES = 512;
my_int_t jump;

void
init(int* arr)
{
  for(int i = 0; i < NUMPAGES*jump; i += jump){
    arr[i] = 0;
  }
}

int main(int argc, char const *argv[])
{
  if (argc < 3){
    printf("usage: tlb pagesize numpages\n");
    exit(0);
  }

  PGSIZE = atoi(argv[1]);
  NUMPAGES = atoi(argv[2]);
  jump = PGSIZE / sizeof(int);

  int* arr = malloc(NUMPAGES*jump*(sizeof(int)));

  init(arr);

  int r;
  time_t starttime, endtime, meantime;

  r = meantime = 0;
  while(r < CYCLES){
    starttime = NANO_SEC(uptime());
    for(int i = 0; i < NUMPAGES*jump; i += jump){
      arr[i] = 1;
    }
    endtime = NANO_SEC(uptime());
    
    meantime += (endtime - starttime);
    r++;
  }

  meantime /= CYCLES;
  
  printf("num of pages = %l access time mean = %l\n", NUMPAGES, meantime);

  free(arr);
  exit(0);
}