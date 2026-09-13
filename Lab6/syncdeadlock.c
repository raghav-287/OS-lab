#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef struct {
    uint locked;
} spinlock_t;

void acquire(spinlock_t *l) {
    while (__sync_lock_test_and_set(&l->locked, 1) != 0);
}

void release(spinlock_t *l) {
    __sync_lock_release(&l->locked);
}

int process_reqs[5][2] = {
    {0, 1},
    {1, 2},
    {0, 2},
    {0, 1},
    {0, 2}
};

void simulate_process(int pid, spinlock_t *res_locks) {
    int r1 = process_reqs[pid][0];
    int r2 = process_reqs[pid][1];

    for (int cycle = 1; cycle <= 2; cycle++) {
        printf("[P%d] Cycle %d: Requesting Resource %d...\n", pid, cycle, r1);
        acquire(&res_locks[r1]);
        printf("[P%d] Cycle %d: GRANTED Resource %d.\n", pid, cycle, r1);

        printf("[P%d] Cycle %d: Requesting Resource %d...\n", pid, cycle, r2);
        acquire(&res_locks[r2]);
        printf("[P%d] Cycle %d: GRANTED Resource %d.\n", pid, cycle, r2);

        printf("--> [P%d] WORKING with Resources %d and %d...\n", pid, r1, r2);
        for (volatile int i = 0; i < 1000000; i++);

        printf("[P%d] Cycle %d: Releasing Resources %d and %d.\n", pid, cycle, r1, r2);
        release(&res_locks[r2]);
        release(&res_locks[r1]);

        for (volatile int i = 0; i < 500000; i++);
    }
    exit(0);
}

int main() {
    spinlock_t res_locks[3];
    for (int i = 0; i < 3; i++) res_locks[i].locked = 0;

    printf(" Multi-Resource Execution \n");

    for (int i = 0; i < 5; i++) {
        int pid = fork();
        if (pid == 0) {
            simulate_process(i, res_locks);
        }
    }

    for (int i = 0; i < 5; i++) {
        wait(0);
    }

    printf("\nAll processes completed successfully without deadlock!\n");
    exit(0);
}