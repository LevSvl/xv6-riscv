#include "syscall.h"
.section dummysec
.globl dummy
dummy:
      li a7, SYS_cancel
      ecall
      ret

spin:
      j spin