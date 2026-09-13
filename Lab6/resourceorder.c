#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef struct {
    uint locked;
} custom_lock_t;

void init_lock(custom_lock_t *l) {
    l->locked = 0;
}

void acquire_lock(custom_lock_t *l) {
    while (__sync_lock_test_and_set(&l->locked, 1) != 0) {
    }
}

void release_lock(custom_lock_t *l) {
    __sync_lock_release(&l->locked);
}

void busy_delay() {
    for (volatile int i = 0; i < 2000000; i++) {}
}

void run_bad_scenario() {
    printf("Running Unordered Lock Acquisition (Deadlock Risk)\n");
    custom_lock_t lock1, lock2;
    init_lock(&lock1);
    init_lock(&lock2);

    int pid = fork();
    if (pid == 0) {
        printf("[Process A] Attempting Lock1...\n");
        acquire_lock(&lock1);
        printf("[Process A] Acquired Lock1.\n");

        busy_delay();

        printf("[Process A] Attempting Lock2...\n");
        acquire_lock(&lock2);
        printf("[Process A] Acquired Lock2.\n");

        release_lock(&lock2);
        release_lock(&lock1);
        exit(0);
    } else {
        printf("[Process B] Attempting Lock2...\n");
        acquire_lock(&lock2);
        printf("[Process B] Acquired Lock2.\n");

        busy_delay();

        printf("[Process B] Attempting Lock1...\n");
        acquire_lock(&lock1);
        printf("[Process B] Acquired Lock1.\n");

        release_lock(&lock1);
        release_lock(&lock2);
        wait(0);
    }
}

void run_fixed_scenario() {
    printf("\nRunning Hierarchical Lock Acquisition (Fixed)\n");
    custom_lock_t lock1, lock2;
    init_lock(&lock1);
    init_lock(&lock2);

    int pid = fork();
    if (pid == 0) {
        printf("[Process A] Attempting Lock1...\n");
        acquire_lock(&lock1);
        printf("[Process A] Acquired Lock1.\n");

        busy_delay();

        printf("[Process A] Attempting Lock2...\n");
        acquire_lock(&lock2);
        printf("[Process A] Acquired Lock2.\n");

        printf("[Process A] Work completed successfully.\n");
        release_lock(&lock2);
        release_lock(&lock1);
        exit(0);
    } else {
        printf("[Process B] Attempting Lock1...\n");
        acquire_lock(&lock1);
        printf("[Process B] Acquired Lock1.\n");

        busy_delay();

        printf("[Process B] Attempting Lock2...\n");
        acquire_lock(&lock2);
        printf("[Process B] Acquired Lock2.\n");

        printf("[Process B] Work completed successfully.\n");
        release_lock(&lock2);
        release_lock(&lock1);
        wait(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "fix") == 0) {
        run_fixed_scenario();
    } else {
        run_bad_scenario();
    }
    exit(0);
}