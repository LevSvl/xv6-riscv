#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];

struct spinlock tid_lock;

extern void freeproc(struct proc *p);
extern void forkret(void);

extern struct spinlock wait_lock;

// Update p->thread_count and make
// it equal to value of process pp.
// tid_lock pp->lock must be held to 
// avoid race between threads
void
thread_cnt_update(struct proc *pp)
{
  struct proc *t;

  for(t = proc; t < &proc[NPROC]; t++){
    if(t->pid == pp->pid && t->tid != pp->tid && t != pp){
      acquire(&t->lock);
      t->thread_count = pp->thread_count;
      release(&t->lock);
    }
  }
}

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

  // hold pp->lock because other threads 
  // can change thread_count concurently
  acquire(&pp->lock);

  // hold tid_lock to make sure that
  // other threads wont have same tid
  acquire(&tid_lock);

  p->pid = pp->pid;
  p->tid = pp->tid + 1;
  pp->thread_count += 1;
  p->thread_count = pp->thread_count;

  release(&tid_lock);
  release(&pp->lock);

  // since pp->lock unlocked and new thread p has actual
  // value of thread_count we can update it broadcast
  acquire(&tid_lock);
  thread_cnt_update(p);
  release(&tid_lock);
  
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

  // Set up new trapframe for thread
  if(mappages(p->pagetable, TRAPFRAME + PGSIZE*p->tid, PGSIZE,
                (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
      uvmunmap(p->pagetable, TRAPFRAME + PGSIZE*p->tid, 1, 0);
      freeproc(p);
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
  np->trapframe->sp = stack + PGSIZE;

  // increment reference counts on open file descriptors.
  for(int i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

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
