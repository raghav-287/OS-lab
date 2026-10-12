#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

volatile int counter = 0;

void increment(void *arg) {
    for (int i = 0; i < 100000; i++) {
        counter++;
    }
    exit(0); // xv6-riscv exit() requires an integer argument
}

int main(void) {
    void *stacks[3];
    
    // Create 3 threads
    for(int i = 0; i < 3; i++) {
        stacks[i] = malloc(4096);
        clone(increment, 0, stacks[i]);
    }
    
    // Wait for all 3 threads to finish
    for(int i = 0; i < 3; i++) {
        join();
    }
    
    printf("Final counter: %d\n", counter);
    exit(0);
}