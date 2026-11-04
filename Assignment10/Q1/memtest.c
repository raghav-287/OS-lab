#include "kernel/types.h"
#include "user/user.h"

#define PGSIZE 4096
#define TEST_PAGES 3

int
main(void)
{
  volatile char *memory;

  printf("Initial allocator statistics:\n");
  memstats();

  memory = (volatile char *)sbrk(TEST_PAGES * PGSIZE);
  if (memory == (char *)-1) {
    printf("memtest: sbrk allocation failed\n");
    exit(1);
  }

  for (int i = 0; i < TEST_PAGES; i++)
    memory[i * PGSIZE] = (char)i;

  printf("After allocating and touching %d pages:\n", TEST_PAGES);
  memstats();

  if (sbrk(-TEST_PAGES * PGSIZE) == (char *)-1) {
    printf("memtest: sbrk release failed\n");
    exit(1);
  }

  printf("After releasing %d pages:\n", TEST_PAGES);
  memstats();
  exit(0);
}
