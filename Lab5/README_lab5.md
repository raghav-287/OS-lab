# XV6 Synchronization Primitives & Shared Memory Implementation

This repository contains the implementation of a shared memory system call interface and four classic process synchronization problems in xv6-riscv:
1. **Peterson’s Algorithm** (`peterson.c`)
2. **Producer-Consumer Problem** (`prodcons.c`)
3. **Readers-Writers Problem** (`readwrite.c`)
4. **Dining Philosophers Problem** (`dining.c`)

---

## Design & System Architecture

### 1. Shared Memory System Calls
To allow processes to share state without file I/O overhead, a static kernel page (`shm_page`) is mapped directly into participating processes at virtual address `0x60000000`.

* **`shm_get()` (Syscall 22):** Maps the shared static kernel page into the process page table at `0x60000000` with user read/write permissions.
* **`shm_unmap()` (Syscall 23):** Unmaps virtual address `0x60000000` from the process page table using `uvmunmap(pagetable, va, 1, 0)`. 
  * *Critical Design Detail:* Passing `do_free = 0` prevents `uvmunmap` from passing kernel static memory to `kfree()`. This eliminates the `panic: freewalk: leaf` crash when child processes call `exit()`.

### 2. Synchronization Mechanisms

* **Peterson’s Algorithm (`peterson.c`):** Enforces 2-process mutual exclusion for a shared counter using `flag[2]` and `turn` variables in shared memory. Memory fences (`__sync_synchronize()`) prevent compiler and CPU memory reordering across RISC-V cores.
* **Producer-Consumer (`prodcons.c`):** Implements a 5-slot circular buffer in shared memory. The producer blocks when the buffer is full (`count == 5`), and the consumer blocks when empty (`count == 0`). Buffer updates use atomic operations to maintain consistency.
* **Readers-Writers (`readwrite.c`):** Uses reader-count synchronization with `__sync_fetch_and_add` to permit concurrent reader processes while maintaining strict mutual exclusion for writer processes.
* **Dining Philosophers (`dining.c`):** Solves the 5-philosopher problem using atomic test-and-set locks (`__sync_lock_test_and_set`). **Deadlock Prevention:** Implements asymmetric chopstick acquisition (even-numbered philosophers pick left chopstick first; odd-numbered pick right chopstick first), breaking the circular wait condition.

---

## Repository & Kernel Modifications

* **`sysproc.c`**: Contains implementations for `sys_shm_get()` and `sys_shm_unmap()`.
* **`syscall.h` & `syscall.c`**: Registered syscall numbers `SYS_shm_get` (23) and `SYS_shm_unmap` (24).
* **`user.h` & `usys.pl`**: Declared user-level stubs for system call invocation.
* **`Makefile`**: Configured `$U/_peterson`, `$U/_prodcons`, `$U/_readwrite`, and `$U/_dining` under `UPROGS`.

---

## How to Build and Run

### 1. Build XV6 Kernel

Clean any prior build artifacts and boot xv6 inside QEMU:

```bash
make clean
make qemu
