#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct shared_data {
    volatile int flag[2];
    volatile int turn;
    volatile int shared_counter;
};

void delay() {
    for (volatile int i = 0; i < 500000; i++);
}

int main(void) {
    int pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(1);
    }


    struct shared_data *sd = (struct shared_data *)shm_get();
    if (sd == (void*)0) {
        printf("Shared memory allocation failed\n");
        exit(1);
    }


    if (pid > 0) {
        sd->flag[0] = 0;
        sd->flag[1] = 0;
        sd->turn = 0;
        sd->shared_counter = 0;
    } else {
        pause(2);
    }

    int id = (pid == 0) ? 1 : 0;
    int other = 1 - id;

    for (int i = 0; i < 10; i++) {
        // Entry Section
        sd->flag[id] = 1;
        sd->turn = other;
        __sync_synchronize();

        while (sd->flag[other] == 1 && sd->turn == other) {
            pause(1);
        }

        // Critical Section
        int current = sd->shared_counter;
        delay();
        sd->shared_counter = current + 1;
        printf("Process %d in CS, counter = %d\n", id, sd->shared_counter);

        // Exit Section
        sd->flag[id] = 0;
        __sync_synchronize();

        // Remainder Section
        delay();
    }

    if (pid > 0) {
        wait(0);
        printf("Final shared_counter value: %d\n", sd->shared_counter);
    }
    shm_unmap();
    exit(0);
}