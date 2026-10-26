#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int main(void) {
  struct meminfo mi1, mi2;
  getmeminfo(&mi1);

  // Request 32 MB lazily
  char *buf = sbrk(32 * 1024 * 1024);
  getmeminfo(&mi2);
  printf("Requested 32MB sbrk. Free pages before: %d, after: %d\n", 
         (int)mi1.free_pages, (int)mi2.free_pages);

  // Touch sparse addresses to trigger faults on demand
  buf[0] = 'A';
  buf[8 * 1024 * 1024] = 'B';
  buf[16 * 1024 * 1024] = 'C';

  getmeminfo(&mi2);
  printf("Free pages after touching 3 pages: %d\n", (int)mi2.free_pages);

  exit(0);
}