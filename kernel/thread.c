#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct cpu cpus[NCPU];

extern struct spinlock pid_lock;

extern struct proc proc[NPROC];

struct spinlock tid_lock;

extern void freeproc(struct proc *p);
extern void forkret(void);

extern struct spinlock wait_lock;


static struct proc*
allocthread(void)
{
  struct proc *p, *pp;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  pp = myproc();

  acquire(&pp->lock);
  p->pid = pp->pid;
  p->tid = pp->tid + 1;
  release(&pp->lock);

  p->state = USED;

  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Parent process/thread user page table.
  p->pagetable = pp->pagetable;
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;
  
  return p;
}

int
clone(uint64 fn, uint64 arg1, uint64 arg2, uint64 stack)
{
  int tid;
  struct proc *np;
  struct proc *p = myproc();
  
  // Allocate thread as process.
  if((np = allocthread()) == 0){
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Child's args
  np->trapframe->a0 = arg1;
  np->trapframe->a1 = arg2;
  
  // Child start execution from it's own ra
  np->trapframe->epc = fn;

  // Child has own stack
  np->trapframe->sp = stack + PGSIZE; // initial stack pointer
  uint64 *pte = walk(p->pagetable,(uint64)stack+PGSIZE, 0);
  printf("pte: %p\n", pte);

  safestrcpy(np->name, p->name, sizeof(p->name));

  tid = np->tid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  return tid;
}

int
join(uint64 stack)
{
  return 0;
}
