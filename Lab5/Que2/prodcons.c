#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 20

struct shared_buffer {
    int buffer[BUFFER_SIZE];
    volatile int in;
    volatile int out;
    volatile int count;
    volatile int turn; // 0 for producer, 1 for consumer slot access
};

void delay() {
    for (volatile int i = 0; i < 300000; i++);
}

int main(void) {
    int pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(1);
    }

    struct shared_buffer *sb = (struct shared_buffer *)shm_get();
    if (sb == (void*)0) {
        printf("Shared memory allocation failed\n");
        exit(1);
    }

    if (pid > 0) {
        // Parent = Producer
        sb->in = 0;
        sb->out = 0;
        sb->count = 0;
        sb->turn = 0;

        for (int i = 1; i <= TOTAL_ITEMS; i++) {
            while (sb->count == BUFFER_SIZE) {
                pause(1);
            }
            //buffer is full
            sb->buffer[sb->in] = i;
            printf("Producer produced: %d\n", i);
            sb->in = (sb->in + 1) % BUFFER_SIZE;
            
            __sync_synchronize();
            sb->count++;
            
            delay();
        }

        wait(0);
        printf("Producer finished.\n");
    } else {
        // Child = Consumer
        pause(2); // Give parent initialization time

        for (int i = 1; i <= TOTAL_ITEMS; i++) {
            // Wait while buffer is empty
            while (sb->count == 0) {
                pause(1);
            }

            int item = sb->buffer[sb->out];
            printf("Consumer consumed: %d\n", item);
            sb->out = (sb->out + 1) % BUFFER_SIZE;
            
            __sync_synchronize();
            sb->count--;
            
            delay();
        }

        printf("Consumer finished.\n");
    }


    shm_unmap();
    exit(0);
}