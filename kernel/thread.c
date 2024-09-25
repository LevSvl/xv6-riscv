#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

int
clone(uint64 fn, uint64 arg1, uint64 arg2, uint64 stack)
{
  return 0;
}

int
join(uint64 stack)
{
  return 0;
}
