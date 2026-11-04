#include "kernel/types.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  volatile char *page;
  int child, status;

  page = (volatile char *)sbrk(PGSIZE);
  if (page == (char *)-1) {
    printf("pagetest: sbrk allocation failed\n");
    exit(1);
  }
  page[0] = 'R';

  printf("Before protection:\n");
  if (pageinfo((uint64)page) < 0) {
    printf("pagetest: pageinfo failed for allocated page\n");
    exit(1);
  }
  if (setreadonly((uint64)page) < 0) {
    printf("pagetest: could not make page read-only\n");
    exit(1);
  }
  printf("After setreadonly:\n");
  pageinfo((uint64)page);

  child = fork();
  if (child < 0) {
    printf("pagetest: fork failed\n");
    exit(1);
  }
  if (child == 0) {
    printf("Child attempting a write to the read-only page...\n");
    page[0] = 'W';
    printf("pagetest: write unexpectedly succeeded\n");
    exit(1);
  }

  if (wait(&status) != child) {
    printf("pagetest: wait failed\n");
    exit(1);
  }
  if (status != -1) {
    printf("pagetest: child was not terminated by the protection fault\n");
    exit(1);
  }
  printf("Parent: child was terminated after the protection fault.\n");
  sbrk(-PGSIZE);
  exit(0);
}
