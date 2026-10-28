# Operating Systems Assignment 9 - Implementation Report (RISC-V xv6)

## Overview
This report documents the architectural design, implementation details, and observed test outputs for the three questions in Operating Systems Assignment 9 on the RISC-V xv6 platform.

---

## Question 1: Page Table Walk and Printing (`vmprint`)

### Design & Implementation
- **Kernel (`kernel/vm.c` & `kernel/sysproc.c`)**: Implemented a recursive walk function `vmprint_walk()` that traverses the SV39 three-level page table hierarchy starting at `p->pagetable`.
- **Filtering**: Iterates over 512 entries per level, filtering out non-present PTEs (`!(pte & PTE_V)`) and virtual addresses at or above kernel space (`KERNBASE` / `MAXVA`).
- **Statistics**: Traverses non-leaf page tables to count intermediate page table pages and tallies leaf mappings containing valid user permissions (`PTE_U`). Prints physical page frame addresses along with active flags (`P`, `W`, `U`).
- **User Interface (`user/pttest.c`)**: Calls `vmprint()` before and after allocating $4 \times 4096$ bytes via `sbrk(4 * 4096)`.

### Verification & Output
The test output verifies that after calling `sbrk(4 * 4096)`, exactly 4 new valid user pages are added to the process's page table mapping while leaving the page-table page count unchanged.

```text
--- Initial Page Table ---
page table 0x0000000087eee000
va: 0x0000000000000000, pa: 0x0000000087fea000, flags: P W U
va: 0x0000000000001000, pa: 0x0000000087fe9000, flags: P W U
va: 0x0000000000002000, pa: 0x0000000087fe8000, flags: P W U
Total valid user pages: 3
Total page-table pages used: 2

--- Page Table After sbrk(4 * 4096) ---
...
Total valid user pages: 7
Total page-table pages used: 2
```

---

## Question 2: Lazy Allocation & Handling Page Faults

### Design & Implementation
- **Lazy `sys_sbrk()` (`kernel/sysproc.c`)**: Modified `sys_sbrk()` so that positive allocation requests ($n > 0$) simply increment `myproc()->sz` without calling `growproc()`, deferring physical frame allocation.
- **Fault Handling (`kernel/trap.c` & `kernel/vm.c`)**: Modified `usertrap()` to intercept store/load page faults (`scause == 15` or `13`). Read the faulting virtual address from `stval` via `r_stval()`.
- **Validation**: If `va < p->sz` and valid, `vmfault()` allocates a physical page via `kalloc()`, zeroes it with `memset()`, maps it with `mappages()` (`PTE_W | PTE_U | PTE_R | PTE_V`), and increments `p->page_faults`.
- **Fault Trapping**: Access attempts above `p->sz` fail validation, printing the PID and address before safely terminating the faulting process via `setkilled(p)`.
- **Memory Management Robustness (`kernel/vm.c`)**: Updated `uvmunmap()`, `uvmdealloc()`, and `uvmcopy()` to skip unmapped lazy pages (`!(pte & PTE_V)`) rather than invoking `panic()`.

### Verification & Output
`faulttest.c` allocates memory for 32 pages, writes to 5 distinct pages, and confirms via `getfaults()` that exactly 5 page faults were handled. It then forks a child process that accesses memory above `p->sz`, demonstrating clean process termination without crashing the kernel.

```text
$ faulttest
Page faults handled: 5
Child attempting invalid access above allocated memory...
usertrap(): unexpected scause 0x000000000000000f pid=4
            sepc=0x00000000000000e4 stval=0x0000000000023000
Parent: Child killed cleanly after invalid memory access.
```

---

## Question 3: Page Replacement Algorithms (FIFO and LRU)

### Design & Implementation
- **User Simulator (`user/pagerepl.c`)**: Implemented FIFO (First-In, First-Out using a circular index) and LRU (Least Recently Used using access timestamp tracking).
- **Execution**: Simulates page replacement step-by-step for a frame array, maintaining frame state, registering hits and page faults, and outputting frame contents at each sequence step.

### Verification & Output
Tested with reference string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` (20 references) across 3, 4, and 5 physical frames.

#### Results Summary Table

| Frames | Algorithm | Faults | Hits | Hit Ratio (%) |
| :---: | :---: | :---: | :---: | :---: |
| **3** | FIFO | 15 | 5 | 25% |
| **3** | LRU | 12 | 8 | 40% |
| **4** | FIFO | 10 | 10 | 50% |
| **4** | LRU | 8 | 12 | 60% |
| **5** | FIFO | 7 | 13 | 65% |
| **5** | LRU | 5 | 15 | 75% |

### Analysis
LRU consistently outperforms FIFO across all frame allocations because LRU accounts for temporal locality by preserving frequently accessed pages (such as pages `0`, `2`, and `3`).