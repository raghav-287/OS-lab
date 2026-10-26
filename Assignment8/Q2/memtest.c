#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

void print_stats(char *label) {
  struct meminfo mi;
  getmeminfo(&mi);
  printf("[%s] Total: %d, Free: %d, User: %d, PT: %d\n",
         label, (int)mi.total_pages, (int)mi.free_pages, 
         (int)mi.proc_user_pages, (int)mi.proc_pt_pages);
}

int main(void) {
  print_stats("Initial");

  char *p = sbrk(4 * 4096);
  p[0] = 1; // trigger fault/allocation
  print_stats("After sbrk(+16KB)");

  int pid = fork();
  if(pid == 0){
    print_stats("Inside Child");
    exit(0);
  } else {
    wait(0);
    print_stats("Child Exited");
  }

  sbrk(-4 * 4096);
  print_stats("After sbrk(-16KB)");
  exit(0);
}