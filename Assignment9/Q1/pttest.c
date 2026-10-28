#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("--- Initial Page Table ---\n");
  vmprint();

  // Allocate 4 pages
  sbrk(4 * 4096);

  printf("\n--- Page Table After sbrk(4 * 4096) ---\n");
  vmprint();

  exit(0);
}