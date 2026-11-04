#include "kernel/types.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  volatile char *memory;
  uint64 first_pa, second_pa, missing_pa;

  memory = (volatile char *)sbrk(2 * PGSIZE);
  if (memory == (char *)-1) {
    printf("vttest: sbrk allocation failed\n");
    exit(1);
  }
  memory[0] = 'A';
  memory[PGSIZE] = 'B';

  first_pa = vtop((uint64)(memory + 7));
  second_pa = vtop((uint64)(memory + PGSIZE + 11));
  missing_pa = vtop((uint64)(memory + 4 * PGSIZE));

  if (first_pa == (uint64)-1) {
    printf("vttest: first allocated page was not mapped\n");
    exit(1);
  }
  if (second_pa == (uint64)-1) {
    printf("vttest: second allocated page was not mapped\n");
    exit(1);
  }
  printf("VA %p -> PA %p\n", memory + 7, (void *)first_pa);
  printf("VA %p -> PA %p\n", memory + PGSIZE + 11, (void *)second_pa);
  printf("Unmapped VA %p -> ", memory + 4 * PGSIZE);
  if (missing_pa == (uint64)-1)
    printf("not mapped (expected)\n");
  else
    printf("unexpected PA %p\n", (void *)missing_pa);

  sbrk(-2 * PGSIZE);
  exit(0);
}
