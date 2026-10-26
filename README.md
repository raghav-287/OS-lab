# Operating System Lab Assignment 8: Memory Management in xv6 (RISC-V)

---

## 1. Design Decisions and Key Data Structures

### Question 1: Virtual to Physical Address Translation (`va2pa`)
* **Design Decision:** The system call `va2pa` exposes internal page table translations to user space. It un-statics `walk()` from `kernel/vm.c` to traverse the 3-level Sv39 page table tree (`L2`, `L1`, `L0`).
* **Validation:** Returns `-1` if the translation fails, if the page table entry (PTE) is not present (`PTE_V` is cleared), or if user access privileges are missing (`PTE_U` is cleared).
* **Key Macros/Data Structures:**
  * `PX(level, va)`: Extracts index for levels 2, 1, and 0 from virtual address `va`.
  * `PTE2PA(pte)`: Extracts physical address from entry.
  * `PTE_FLAGS(pte)`: Extracts flag bits (`PTE_V`, `PTE_R`, `PTE_W`, `PTE_U`).

### Question 2: Physical Memory Statistics (`getmeminfo`)
* **Design Decision:** Physical frame accounting is maintained inside `kernel/kalloc.c` using a global `free_pages` counter protected under `kmem.lock` to prevent race conditions during concurrently executed allocations or frees.
* **Page Table Overhead:** Added `count_pt_pages()` in `kernel/vm.c` to recursively traverse and count valid frame tables allocated for process page directories (Levels 2, 1, and 0).
* **Key Data Structures:**
  ```c
  struct meminfo {
    uint64 total_pages;      // Total system RAM frames ((PHYSTOP - KERNBASE) / PGSIZE)
    uint64 free_pages;       // Tracked dynamically in kalloc.c
    uint64 proc_user_pages;  // p->sz / PGSIZE
    uint64 proc_pt_pages;    // Allocated frames backing page tables
  };
  ```

### Question 3: Process Page Table Tree Dump (`vmprint`)
* **Design Decision:** Hierarchical printing mechanism in `kernel/vm.c` that displays entries below `KERNBASE`. Standardized visual levels using prefix indentations (`..` for L2, `.. ..` for L1, `.. .. ..` for L0).
* **Stack Guard Identification:** Exec creates a guard page with `PTE_U` cleared directly below the stack page. `vmprint` accurately reflects this entry with `U=0`.

### Question 4: Lazy Memory Allocation (Demand Paging)
* **Design Decision:** Deferred physical allocation for heap growth.
  * `sys_sbrk(n)` ($n > 0$): Only increments `myproc()->sz` without invoking `growproc()`.
  * `sys_sbrk(n)` ($n < 0$): Immediately deallocates pages via `growproc()`.
  * Trap Handling: Captures RISC-V page fault exception vectors `scause == 13` (Load Page Fault) and `scause == 15` (Store/Instruction Page Fault) in `usertrap()`.
  * Address Validation: Validates fault address $va \in [\text{PGSIZE}, p->sz)$. Faults outside this range, below the guard page, or on `kalloc()` failures cleanly terminate offending processes with `p->killed = 1`.
  * Fork Safety: Modified `uvmcopy()` in `kernel/vm.c` to skip unmapped lazy pages (`PTE_V == 0`) rather than triggering kernel panic.

---

## 2. Problems Faced and Resolution

1. **Kernel Panic on `fork()` with Lazy Heap Pages:**
   * *Problem:* When a process lazily expanded its heap and called `fork()`, `uvmcopy()` attempted to copy non-existent physical frames, causing a kernel panic.
   * *Resolution:* Modified `uvmcopy()` to inspect `walk(old, i, 0)`. If `PTE_V` is cleared, `uvmcopy()` skips the unallocated virtual page cleanly without raising errors.

2. **Race Conditions on Free Page Counter:**
   * *Problem:* Concurrent memory allocation in multi-core execution resulted in inaccurate statistics reported by `getmeminfo`.
   * *Resolution:* Enforced lock-protected access to `kmem.free_pages` inside `kalloc()` and `kfree()` while holding `kmem.lock`.

3. **System Call Data Passing across Kernel-User Boundary:**
   * *Problem:* User-space applications passing pointer addresses for `meminfo` directly caused fault checks in 64-bit mode.
   * *Resolution:* Used kernel helper `copyout(p->pagetable, uaddr, (char *)&mi, sizeof(mi))` to safely copy memory stats from kernel context to user address space.

---

## 3. Memory Usage Before & After (Free Pages Analysis)

### Question 2 Memory Profile (`memtest`)
| Stage | Free Pages | User Pages | Page Table Pages | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Initial State** | `32495` | `3` | `3` | Process initialized with code, guard page, stack. |
| **After sbrk(+16KB)** | `32495` | `7` | `3` | Heap requested; lazy allocation leaves free physical frames unchanged. |
| **Inside Child (Post-Fault)** | `32488` | `7` | `3` | `fork()` duplicates active physical frames for child process. |
| **Child Exited & Reaped** | `32495` | `7` | `3` | Child physical pages reclaimed by `kfree()`. Zero leak observed. |
| **After sbrk(-16KB)** | `32499` | `3` | `3` | Explicit heap shrinking releases physical frames. |

### Question 4 Lazy Memory Profile (`lazytest`)
* **Baseline System Free Pages:** `32495`
* **Allocation Request (`sbrk(32 MB)`):** `32495` free pages (*0 physical frames consumed*).
* **Sparse Access (Writing to 3 distant pages):** Decrements free pages from `32495` to `32492` exactly (*3 physical frames allocated on demand*).

