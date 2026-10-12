#include "kernel/types.h"
#include "user/user.h"

void do_work() {
    int i, j;
    // CPU bound loop to simulate heavy workload
    for(i = 0; i < 1000; i++) {
        for(j = 0; j < 1000000; j++) {
            asm volatile("nop"); // Prevent compiler from optimizing this away
        }
    }
}

int main(void) {
    int pid;
    int priorities[4] = {5, 10, 15, 20}; // 4 distinct priorities

    printf("Starting priority scheduling test...\n");

    for(int i = 0; i < 4; i++) {
        pid = fork();
        if(pid < 0) {
            printf("Fork failed\n");
            exit(1);
        }
        if(pid == 0) {
            // Child process
            int my_pid = getpid();
            set_priority(my_pid, priorities[i]);
            printf("Child %d started with priority %d\n", my_pid, priorities[i]);
            
            do_work();
            
            printf("Child %d (priority %d) finished\n", my_pid, priorities[i]);
            exit(0);
        }
    }

    // Parent process waits for all 4 children to finish
    for(int i = 0; i < 4; i++) {
        wait(0);
    }

    printf("Priority test completed.\n");
    exit(0);
}