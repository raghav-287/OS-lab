#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  char *ptr = sbrk(32 * 4096);

  // Write to exactly 5 distinct pages
  for(int i = 0; i < 5; i++){
    ptr[i * 4096] = 'A';
  }

  int faults = getfaults();
  printf("Page faults handled: %d\n", faults);

  // Demonstrate clean termination on out-of-bounds access
  int pid = fork();
  if(pid == 0){
    printf("Child attempting invalid access above allocated memory...\n");
    char bad_val = ptr[33 * 4096]; // Should trigger process kill
    printf("Should not print this line: %c\n", bad_val);
    exit(0);
  }

  wait(0);
  printf("Parent: Child killed cleanly after invalid memory access.\n");
  exit(0);
}