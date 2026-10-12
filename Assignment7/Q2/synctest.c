#include "kernel/types.h"
#include "user/user.h"

volatile int counter = 0;

void increment(void *arg) {
    for (int i = 0; i < 100000; i++) {
        sem_wait(0);
        counter++;
        sem_signal(0);
    }
    exit(0);
}

int main(void) {
    sem_init(0, 1); // Mutex
    void *stacks[3];
    for(int i = 0; i < 3; i++) {
        stacks[i] = malloc(4096);
        clone(increment, 0, stacks[i]);
    }
    for(int i = 0; i < 3; i++) {
        join();
    }
    printf("Final synchronized counter: %d\n", counter);
    exit(0);
}