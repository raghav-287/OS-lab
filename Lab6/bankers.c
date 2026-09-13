#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N 5 // Number of processes
#define M 3 // Number of resource types

int Allocation[N][M] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2}
};

int Max[N][M] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
};

int Available[M] = {3, 3, 2};
int Need[N][M];

void compute_need() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            Need[i][j] = Max[i][j] - Allocation[i][j];
        }
    }
}

int is_safe(int safe_seq[]) {
    int work[M];
    int finish[N];

    for (int i = 0; i < M; i++) work[i] = Available[i];
    for (int i = 0; i < N; i++) finish[i] = 0;

    int count = 0;
    while (count < N) {
        int found = 0;
        for (int p = 0; p < N; p++) {
            if (!finish[p]) {
                int possible = 1;
                for (int j = 0; j < M; j++) {
                    if (Need[p][j] > work[j]) {
                        possible = 0;
                        break;
                    }
                }
                if (possible) {
                    for (int k = 0; k < M; k++)
                        work[k] += Allocation[p][k];
                    safe_seq[count++] = p;
                    finish[p] = 1;
                    found = 1;
                }
            }
        }
        if (!found) return 0; // Unsafe
    }
    return 1; // Safe
}

void request_resources(int p_id, int req[]) {
    printf("\nProcess P%d requesting [%d, %d, %d]\n", p_id, req[0], req[1], req[2]);
    
    for (int j = 0; j < M; j++) {
        if (req[j] > Need[p_id][j]) {
            printf("Error: Process exceeded its maximum claim!\n");
            return;
        }
    }

    for (int j = 0; j < M; j++) {
        if (req[j] > Available[j]) {
            printf("Process P%d must wait. Resources not available.\n", p_id);
            return;
        }
    }

    for (int j = 0; j < M; j++) {
        Available[j] -= req[j];
        Allocation[p_id][j] += req[j];
        Need[p_id][j] -= req[j];
    }

    int safe_seq[N];
    if (is_safe(safe_seq)) {
        printf("Request granted to P%d successfully!\nSafe Sequence: ", p_id);
        for (int i = 0; i < N; i++) {
            printf("P%d ", safe_seq[i]);
        }
        printf("\n");
    } else {
        printf("Request denied: would lead to unsafe state. Rolling back.\n");
        for (int j = 0; j < M; j++) {
            Available[j] += req[j];
            Allocation[p_id][j] -= req[j];
            Need[p_id][j] += req[j];
        }
    }
}

int main(int argc, char *argv[]) {
    compute_need();
    int safe_seq[N];

    printf("Initial Safety Check\n");
    if (is_safe(safe_seq)) {
        printf("System is in a SAFE state.\nInitial Safe Sequence: ");
        for (int i = 0; i < N; i++) printf("P%d ", safe_seq[i]);
        printf("\n");
    } else {
        printf("System is in an UNSAFE state.\n");
    }

    int req1[M] = {1, 0, 2};
    request_resources(1, req1);

    int req2[M] = {3, 3, 0};
    request_resources(4, req2);

    exit(0);
}