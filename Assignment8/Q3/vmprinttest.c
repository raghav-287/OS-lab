#include "kernel/types.h"
#include "user/user.h"

int main(void) {
  printf("=== Initial Page Table Dump ===\n");
  vmprint();

  sbrk(8 * 4096);
  printf("\n=== Dump After sbrk(+32KB) ===\n");
  vmprint();

  sbrk(-4 * 4096);
  printf("\n=== Dump After sbrk(-16KB) ===\n");
  vmprint();

  exit(0);
}