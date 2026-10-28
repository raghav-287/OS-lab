#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

// static char shm_page[PGSIZE] __attribute__((aligned(PGSIZE)));

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
//Assignment 7:
struct semaphore {
  int value;
  struct spinlock lk;
  int active;
} sems[10];

uint64 sys_sem_init(void) {
  int id, val;
  argint(0, &id);
  argint(1, &val);
  initlock(&sems[id].lk, "semaphore");
  sems[id].value = val;
  sems[id].active = 1;
  return 0;
}

uint64 sys_sem_signal(void) {
  int id;
  argint(0, &id);
  acquire(&sems[id].lk);
  sems[id].value++;
  if(sems[id].value <= 0) {
    wakeup(&sems[id]);
  }
  release(&sems[id].lk);
  return 0;
}

extern int clone(uint64, uint64, uint64);
extern int join(void);

uint64 sys_clone(void) {
  uint64 fcn, arg, stack;
  argaddr(0, &fcn);
  argaddr(1, &arg);
  argaddr(2, &stack);
  return clone(fcn, arg, stack);
}

uint64 sys_join(void) {
  return join();
}

uint64 sys_set_priority(void) {
  int pid, priority;
  argint(0, &pid);
  argint(1, &priority);
  return set_priority(pid, priority);
}

uint64 sys_get_priority(void) {
  int pid;
  argint(0, &pid);
  return get_priority(pid);
}

uint64 sys_sem_wait(void) {
  int id;
  argint(0, &id);
  acquire(&sems[id].lk);
  sems[id].value--;
  while(sems[id].value < 0) {
    sleep_prepare(&sems[id]);
    release(&sems[id].lk);
    sleep();
    acquire(&sems[id].lk);
  }
  release(&sems[id].lk);
  return 0;
}

uint64
sys_vmprint(void)
{
  struct proc *p = myproc();
  return vmprint(p->pagetable);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  argint(0, &n);
  struct proc *p = myproc();
  addr = p->sz;

  if (n > 0) {
    // Lazy allocation: expand size without immediately allocating physical pages
    p->sz += n;
  } else if (n < 0) {
    p->sz = uvmdealloc(p->pagetable, p->sz, p->sz + n);
  }
  return addr;
}

uint64
sys_getfaults(void)
{
  return myproc()->page_faults;
}