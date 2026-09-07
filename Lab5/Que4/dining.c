#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PHIL 5
#define ITERATIONS 3

struct shared_state {
    volatile int chopsticks[NUM_PHIL]; // 0 = free, 1 = taken
};

void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

void test_philosopher(int id, struct shared_state *state) {
    int left = id;
    int right = (id + 1) % NUM_PHIL;

    for (int i = 0; i < ITERATIONS; i++) {
        printf("Philosopher %d is THINKING\n", id);
        delay(300000);

        printf("Philosopher %d is HUNGRY\n", id);

        // Asymmetric chopstick pickup to prevent deadlock
        if (id % 2 == 0) {
            // Even pick left then right
            while (__sync_lock_test_and_set(&state->chopsticks[left], 1)) {
                pause(1);
            }
            while (__sync_lock_test_and_set(&state->chopsticks[right], 1)) {
                pause(1);
            }
        } else {
            // Odd pick right then left
            while (__sync_lock_test_and_set(&state->chopsticks[right], 1)) {
                pause(1);
            }
            while (__sync_lock_test_and_set(&state->chopsticks[left], 1)) {
                pause(1);
            }
        }

        // Critical Section (Eating)
        printf("Philosopher %d is EATING\n", id);
        delay(400000);

        // Release chopsticks
        __sync_lock_release(&state->chopsticks[left]);
        __sync_lock_release(&state->chopsticks[right]);
    }

    printf("Philosopher %d finished.\n", id);
}

int main(void) {
    struct shared_state *state = (struct shared_state *)shm_get();
    if (state == (void*)0) {
        printf("Shared memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < NUM_PHIL; i++) {
        state->chopsticks[i] = 0;
    }

    // Fork 4 child processes to total 5 philosophers
    int id = 0;
    for (int i = 1; i < NUM_PHIL; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("Fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            id = i;
            // Child re-maps shared memory
            state = (struct shared_state *)shm_get();
            break;
        }
    }

    test_philosopher(id, state);

    if (id == 0) {
        // Parent waits for all 4 children
        for (int i = 1; i < NUM_PHIL; i++) {
            wait(0);
        }
        printf("All philosophers finished eating cleanly.\n");
    }

    shm_unmap();
    exit(0);
}