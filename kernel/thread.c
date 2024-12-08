#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];

struct spinlock common_thread_lock;
struct spinlock futex_lock;


extern void freeproc(struct proc *p);
extern void forkret(void);

extern struct spinlock wait_lock;

extern char dummy[];

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

  if(pp->thread_count > NTHREAD){
    release(&pp->lock);
    release(&p->lock);
    return 0;
  }
  
  // hold tid_lock to make sure that
  // other threads wont have same tid
  acquire(&common_thread_lock);

  p->pid = pp->pid; // child thread has same pid

  // since thread_count initially equal 1,
  // child's tid is the current value of 
  // thread_count before incrementing it
  p->tid = pp->thread_count;
  p->thread_count = ++pp->thread_count;

  release(&common_thread_lock);
  release(&pp->lock);

  // since pp->lock unlocked and new thread p has actual
  // value of thread_count we can update it broadcast
  acquire(&common_thread_lock);
  thread_cnt_update(p);
  release(&common_thread_lock);
  
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
      freeproc(p);
      release(&p->lock);
      return 0;
    }
  p->trapframe_was_mapped = 1;

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

  // Child return address must point to dummy
  np->trapframe->ra = DUMMY;
  
  // Child has own stack
  np->trapframe->sp = stack + PGSIZE;

  // When child thread complete execution,
  // it's epilogue code can restore incorrect 
  // sp by loading s0 from stack, to avoid this
  // initial s0 should contain the same stack 
  np->trapframe->s0 = stack + PGSIZE;

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
cancel(uint64 stack)
{
  struct proc *p = myproc();

  // Close all open files.
  for(int fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd]){
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in join().
  wakeup(p->parent);

  acquire(&p->lock);

  p->state = ZOMBIE;

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie cancel");
}

int
join(uint64 stack, uint64 ret)
{
  struct proc *pp;
  int tid, found;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for(;;){
    found = 0;
    for(pp = proc; pp < &proc[NPROC]; pp++){
      if(pp->trapframe && PGROUNDUP(pp->trapframe->sp) == stack + PGSIZE){
        acquire(&pp->lock);

        found = 1;
        if(pp->state == ZOMBIE){
          tid = pp->tid;
          if(ret != 0 && copyout(p->pagetable, ret, (char*)&pp->trapframe->a0,
                                sizeof(pp->trapframe->a0)) < 0){
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return tid;
        }

        release(&pp->lock);
      }
    }

    if(!found || killed(p)){
      release(&wait_lock);
      return -1;
    }

    sleep(p, &wait_lock);
  }

  return 0;
}

int
futex_wait(uint64 addr, int expected)
{
  int futex_val;
  struct proc *p = myproc(); 

  acquire(&futex_lock);
  
  for(;;){
    if((copyin(p->pagetable, (char*)&futex_val,
                  addr, sizeof(futex_val)) < 0)){
      release(&futex_lock);
      return -1;
    }
    if(futex_val != expected){
      release(&futex_lock);
      return 0;
    }
    sleep((void *)addr, &futex_lock);
  }
}

int
futex_wake(uint64 addr)
{
  acquire(&futex_lock);
  wakeup((void *)addr);
  release(&futex_lock);
  
  return 0;
}
