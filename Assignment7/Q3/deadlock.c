#include "kernel/types.h"
#include "user/user.h"

#define N 5 // Processes
#define M 3 // Resource types

int Available[M] = {3, 3, 2};
int Max[N][M] = { {7,5,3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3} };
int Allocation[N][M] = { {0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2} };
int Request[N][M];

void detect_deadlock() {
    int work[M];
    int finish[N] = {0};
    int deadlocked[N];
    int d_count = 0;

    for(int i = 0; i < M; i++) work[i] = Available[i];

    int found;
    do {
        found = 0;
        for(int i = 0; i < N; i++) {
            if(!finish[i]) {
                int can_satisfy = 1;
                for(int j = 0; j < M; j++) {
                    if(Request[i][j] > work[j]) {
                        can_satisfy = 0; break;
                    }
                }
                if(can_satisfy) {
                    for(int j = 0; j < M; j++) work[j] += Allocation[i][j];
                    finish[i] = 1;
                    found = 1;
                }
            }
        }
    } while(found);

    for(int i = 0; i < N; i++) {
        if(!finish[i]) deadlocked[d_count++] = i;
    }

    if(d_count > 0) {
        printf("Deadlock detected! Processes: ");
        for(int i = 0; i < d_count; i++) printf("%d ", deadlocked[i]);
        printf("\n");
    } else {
        printf("No deadlock detected.\n");
    }
}

int main(void) {
    detect_deadlock();
    exit(0);
}