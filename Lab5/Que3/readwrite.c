#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct rw_lock {
    volatile int readers;
    volatile int writing;
    volatile int shared_data;
};

void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

int main(void) {
    int pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(1);
    }

    struct rw_lock *rw = (struct rw_lock *)shm_get();
    if (rw == (void*)0) {
        printf("Shared memory allocation failed\n");
        exit(1);
    }

    if (pid > 0) {
        // Writer Process
        rw->readers = 0;
        rw->writing = 0;
        rw->shared_data = 100;

        for (int i = 0; i < 5; i++) {
            while (rw->readers > 0 || rw->writing == 1) {
                pause(1);
            }

            rw->writing = 1;
            __sync_synchronize();

            rw->shared_data += 10;
            printf("Writer updated shared_data to: %d\n", rw->shared_data);
            delay(600000);

            rw->writing = 0;
            __sync_synchronize();

            delay(300000);
        }

        wait(0);
        printf("Writer finished.\n");
    } else {
        // Reader Process
        pause(1); // Reduced pause so reader starts concurrently

        for (int i = 0; i < 5; i++) {
            while (rw->writing == 1) {
                pause(1);
            }

            __sync_fetch_and_add(&rw->readers, 1);
            __sync_synchronize();

            printf("Reader read shared_data: %d (Active Readers: %d)\n", rw->shared_data, rw->readers);
            delay(200000);

            __sync_fetch_and_sub(&rw->readers, 1);
            __sync_synchronize();

            delay(400000);
        }

        printf("Reader finished.\n");
    }

    shm_unmap();
    exit(0);
}