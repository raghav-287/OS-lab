#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int global_var = 100;

int main(void) {
  int local_var = 50;
  char *heap_var = malloc(64);

  printf("Global VA: %p -> PA: %p\n", (void*)&global_var, (void*)va2pa((uint64)&global_var));
  printf("Local VA:  %p -> PA: %p\n", (void*)&local_var, (void*)va2pa((uint64)&local_var));
  printf("Heap VA:   %p -> PA: %p\n", (void*)heap_var, (void*)va2pa((uint64)heap_var));
  printf("Unmapped VA: 0x7FFFFFFF -> PA: %d\n", va2pa(0x7FFFFFFF));

  int pid = fork();
  if(pid == 0){
    printf("[Child] Global VA: %p -> PA: %p\n", (void*)&global_var, (void*)va2pa((uint64)&global_var));
    exit(0);
  } else {
    wait(0);
    printf("[Parent] Global VA: %p -> PA: %p\n", (void*)&global_var, (void*)va2pa((uint64)&global_var));
  }
  free(heap_var);
  exit(0);
}