#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
  char *a = (char*)0;
  for(int i = 0; i < 3086; i++)
    printf("%d: %x\n",i, *(a + i));

  return 0;
}
