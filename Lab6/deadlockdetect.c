#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_P 10

int graph[MAX_P][MAX_P];
int visited[MAX_P];
int rec_stack[MAX_P];
int path[MAX_P];
int cycle_start = -1, cycle_end = -1;

int dfs(int v, int num_p) {
    visited[v] = 1;
    rec_stack[v] = 1;

    for (int u = 0; u < num_p; u++) {
        if (graph[v][u]) {
            path[u] = v;
            if (!visited[u]) {
                if (dfs(u, num_p)) return 1;
            } else if (rec_stack[u]) {
                cycle_start = u;
                cycle_end = v;
                return 1;
            }
        }
    }
    rec_stack[v] = 0;
    return 0;
}

void detect_deadlock(int num_p, int num_r, int alloc[][MAX_P], int req[][MAX_P], const char *label) {
    printf(" Testing Scenario: %s\n", label);

    for (int i = 0; i < num_p; i++) {
        visited[i] = 0;
        rec_stack[i] = 0;
        path[i] = -1;
        for (int j = 0; j < num_p; j++) graph[i][j] = 0;
    }

    for (int i = 0; i < num_p; i++) {
        for (int r = 0; r < num_r; r++) {
            if (req[i][r] > 0) {
                for (int j = 0; j < num_p; j++) {
                    if (alloc[j][r] > 0 && i != j) {
                        graph[i][j] = 1;
                    }
                }
            }
        }
    }

    printf("Constructed Wait-For Graph Edges:\n");
    for (int i = 0; i < num_p; i++) {
        for (int j = 0; j < num_p; j++) {
            if (graph[i][j]) {
                printf("  P%d -> P%d\n", i, j);
            }
        }
    }

    cycle_start = -1;
    int has_deadlock = 0;

    for (int i = 0; i < num_p; i++) {
        if (!visited[i]) {
            if (dfs(i, num_p)) {
                has_deadlock = 1;
                break;
            }
        }
    }

    if (has_deadlock) {
        printf("STATUS: DEADLOCK DETECTED!\nDeadlock Cycle: ");
        int curr = cycle_end;
        printf("P%d", cycle_start);
        
        int print_stack[MAX_P];
        int p_idx = 0;
        while (curr != cycle_start) {
            print_stack[p_idx++] = curr;
            curr = path[curr];
        }
        for (int k = p_idx - 1; k >= 0; k--) {
            printf(" -> P%d", print_stack[k]);
        }
        printf(" -> P%d\n", cycle_start);
    } else {
        printf("STATUS: NO DEADLOCK DETECTED.\n");
    }
}

int main() {
    int alloc1[MAX_P][MAX_P] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    int req1[MAX_P][MAX_P] = {
        {0, 1, 0},
        {0, 0, 1},
        {0, 0, 0}
    };
    detect_deadlock(3, 3, alloc1, req1, "No Deadlock (Acyclic)");

    int alloc2[MAX_P][MAX_P] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    int req2[MAX_P][MAX_P] = {
        {0, 1, 0},
        {0, 0, 1},
        {1, 0, 0}
    };
    detect_deadlock(3, 3, alloc2, req2, "Deadlock (3-Process Circular Wait)");

    exit(0);
}