// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

// Assignment 10
struct {
  struct spinlock lock;
  struct run *freelist;
  uint64 free_pages;
  uint64 total_pages;
  uint64 allocated_pages;
  uint64 freed_pages;
  uint64 current_pages;
  int initialized;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  kmem.free_pages = 0;
  kmem.total_pages = 0;
  kmem.allocated_pages = 0;
  kmem.freed_pages = 0;
  kmem.current_pages = 0;
  kmem.initialized = 0;
  freerange(end, (void*)PHYSTOP);
  acquire(&kmem.lock);
  kmem.total_pages = kmem.free_pages;
  kmem.initialized = 1;
  release(&kmem.lock);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.free_pages++;
  if (kmem.initialized) {
    kmem.freed_pages++;
    kmem.current_pages--;
  }
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r) {
    kmem.freelist = r->next;
    kmem.free_pages--;
    kmem.allocated_pages++;
    kmem.current_pages++;
  }
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

void
kalloc_stats(uint64 *total, uint64 *allocated, uint64 *freed,
             uint64 *current, uint64 *available)
{
  acquire(&kmem.lock);
  *total = kmem.total_pages;
  *allocated = kmem.allocated_pages;
  *freed = kmem.freed_pages;
  *current = kmem.current_pages;
  *available = kmem.free_pages;
  release(&kmem.lock);
}
