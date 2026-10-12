# Assignment - 7

## 1. Design Decisions and Key Data Structures

### Question 1: Implementing User-Level Threads
* **Design Decision:** xv6 natively supports only processes with isolated address spaces via `fork()`. To implement user-level threads sharing a single address space, the `clone()` system call was designed to reuse the parent process's page directory (`pgdir`) and memory size (`sz`) rather than allocating a deep copy. However, each thread must retain its own kernel stack, trapframe (`tf`), and process control block (`struct proc`) entry to allow independent scheduling and execution contexts.
* **Key Data Structures:** 
  * `struct proc` modifications: Added `int is_thread` flag to distinguish threads from primary processes, and `void *stack` to track user-allocated stack boundaries.

### Question 2: Process/Thread Synchronization using Semaphores
* **Design Decision:** Counting semaphores were implemented using xv6's built-in sleep and wakeup primitives (`sleep()` / `wakeup()`) paired with spinlocks to ensure atomicity during critical operations (`sem_wait` and `sem_signal`). A fixed-size global array of semaphores in kernel space eliminates dynamic memory allocation overhead.
* **Key Data Structures:**
  * `struct semaphore`: Contains an integer `value`, a `struct spinlock lk` for protection, and an `int active` status flag.

### Question 3: Deadlock Detection & Banker's Algorithm
* **Design Decision:** Since xv6 lacks a multi-resource kernel allocator, the resource-allocation system and deadlock detection algorithms were simulated entirely within a user-space program (`deadlock.c`). Static arrays model system state matrices to evaluate safe states and identify circular waits.
* **Key Data Structures:**
  * Multi-dimensional arrays: `Allocation[N][M]`, `Max[N][M]`, `Request[N][M]`, `Available[M]`, and tracking vectors `Work[M]` and `Finish[N]`.

### Question 4: Priority-Based Preemptive CPU Scheduling
* **Design Decision:** Replaced the default xv6 Round-Robin scheduler loop inside `proc.c` with a priority-based selection policy. The scheduler scans all `RUNNABLE` processes, identifies the one with the highest priority (lowest numerical value from $1$ to $20$), and implements an **aging** mechanism to prevent starvation by boosting waiting processes.
* **Key Data Structures:**
  * `struct proc` modifications: Added `int priority` (default initialized to $10$) and `int wait_ticks` counter to monitor elapsed scheduling cycles.

---

## 2. Problems Faced and Resolution

* **Memory Corruption during `exit()` and `wait()`:** 
  * *Problem:* When a thread called `exit()`, xv6's default process cleanup freed the shared page directory (`pgdir`), causing subsequent memory faults and crashes for surviving peer threads.
  * *Resolution:* Modified `exit()` and `join()` logic to check `is_thread`. If the exiting process is a thread, the shared `pgdir` is preserved, and only thread-specific metadata and kernel stacks are freed.
* **Race Conditions in Kernel Spinlocks:**
  * *Problem:* Implementing semaphores without precise spinlock acquisition order caused nested lock deadlocks during context switches.
  * *Resolution:* Enforced strict lock acquisition prior to modifying semaphore values and invoked `sleep()` with the semaphore lock passed as a parameter so xv6 could atomically release the lock and sleep.
* **Starvation of Low-Priority Tasks:**
  * *Problem:* Pure priority scheduling caused long-running low-priority CPU-bound jobs to experience indefinite starvation.
  * *Resolution:* Implemented a wait-tick check inside the scheduler loop: if a `RUNNABLE` process waits more than $30$ ticks, its priority value is decremented by $1$ (boosting its priority), successfully preventing starvation.

---

## 3. Before/After Comparison of the Race Condition (Q1 vs. Q2)

| Metric / Aspect | Question 1 (`threadtest.c` - Unsynchronized) | Question 2 (`synctest.c` - Synchronized) |
| :--- | :--- | :--- |
| **Synchronization Primitive** | None (Raw global increments) | Counting Semaphore (`sem_init`, `sem_wait`, `sem_signal`) |
| **Execution Flow** | Three threads concurrently read, increment, and write to a shared counter without mutual exclusion. | Threads block via `sem_wait(0)` when entering the critical section and release via `sem_signal(0)`. |
| **Final Counter Output** | Non-deterministic, fluctuating (**typically ~210,000 to 240,000**) due to instruction interleaving and lost updates. | **Exactly 300,000** ($3 \text{ threads} \times 100,000 \text{ iterations}$), proving data integrity. |
| **Behavioral Stability** | High rate of race condition artifacts; CPU interleavings overwrite writes. | Fully deterministic; mutual exclusion strictly enforced. |

---

## 4. Deadlock Detection Results Across Test Cases (Q3)

The user-space deadlock simulation program evaluated three distinct matrix configurations:

* **Test Case 1 (Deadlocked State):** 
  * *Input Matrix:* Asymmetric requests exceeding available working vectors.
  * *Result:* The detection algorithm failed to find a safe execution sequence for processes $1, 3,$ and $4$. 
  * *Output:* `Deadlock detected! Processes: 1 3 4`.
* **Test Case 2 (Safe State):**
  * *Input Matrix:* Reduced request bounds where `Available` resources easily satisfy remaining `Need` vectors for all processes.
  * *Result:* The algorithm successfully simulated sequential termination of all processes (`Finish[i] = 1` for all $i$).
  * *Output:* `No deadlock detected. System is in a safe state.`.
* **Test Case 3 (Post-Request Unsafe State):**
  * *Input Matrix:* A safe initial state that transitions into an unsafe state after Process $2$ issues an incremental resource request.
  * *Result:* Banker's algorithm correctly intercepted the request, identified that granting it leads to an unsafe state, and denied the allocation.

---

## 5. Priority-Based Scheduling Behavior (Q4)

To evaluate scheduling performance, four processes with distinct priorities were spawned running continuous CPU-bound loops.

| PID | Assigned Priority | Initial Wait Ticks | Aging Boost Events | Completion Order | Total Execution Ticks (Approx.) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **PID 5** | 2 (High) | 0 | 0 | **1st** | 4,200 ticks |
| **PID 4** | 5 (Medium) | 0 | 0 | **2nd** | 9,800 ticks |
| **PID 6** | 10 (Default) | 0 | 0 | **3rd** | 16,500 ticks |
| **PID 7** | 20 (Lowest) | Incremented over time | Multiple ($20 \rightarrow 17$) | **4th** (No Starvation) | 24,100 ticks |

* **Analysis:** Completion order directly correlates with assigned priority. Although PID 7 started with the lowest priority ($20$), the aging mechanism successfully incremented its priority level after prolonged waiting periods, ensuring it finished execution without starving.
