#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
  printf("%d\n", ugetpid());
  return 0;
}