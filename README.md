# Operating Systems Assignment 10

This solution targets the repository's RISC-V xv6 version. The provided PDF
uses the older x86 name `walkpgdir`; its equivalent here is `walk()`, which
walks the Sv39 page table. User addresses and physical addresses are `uint64`
on this target.

## Implementation

### Q1 — Physical memory statistics

`kalloc.c` maintains lock-protected counts for allocator-managed pages,
successful allocations, runtime frees, outstanding allocations, and available
pages. Pages inserted into the freelist during `kinit()` are excluded from the
runtime free count. `memstats()` snapshots the values and prints them from the
kernel. `Q1/memtest.c` touches three `sbrk()` pages to force physical allocation,
prints the statistics, releases the pages, and prints the updated values.

### Q2 — Virtual-to-physical translation

The `vtop()` system call uses `walk()` to find a valid user PTE, extracts its
physical page address, and adds the virtual page offset. It returns `(uint64)-1`
for an invalid or unmapped address. `Q2/vttest.c` touches two allocated pages,
translates addresses with nonzero offsets, and checks an unmapped address.

### Q3 — Page inspection and protection

`pageinfo()` prints the physical address, frame number, and `P`, `W`, and `U`
flags, or reports an unmapped address. `setreadonly()` clears `PTE_W` for a
valid user mapping and flushes the local TLB. `Q3/pagetest.c` checks the mapping,
protects it, then forks a child to attempt a write. The existing user-trap
failure path terminates the child on the resulting store-page fault, leaving
the parent alive to report the result.

### Q4 — Replacement algorithms

`Q4/pagereplace.c` simulates FIFO, LRU, and Optimal replacement for the
specified 20-reference string. It prints each frame state and hit/fault for
three frames, then compares all three algorithms with 3, 4, and 5 frames.
Optimal evicts the resident page whose next use is farthest in the known
reference string.

| Frames | Algorithm | Faults | Hits | Hit ratio | Fault ratio |
|---:|:---|---:|---:|---:|---:|
| 3 | FIFO | 15 | 5 | 25% | 75% |
| 3 | LRU | 12 | 8 | 40% | 60% |
| 3 | Optimal | 9 | 11 | 55% | 45% |
| 4 | FIFO | 10 | 10 | 50% | 50% |
| 4 | LRU | 8 | 12 | 60% | 40% |
| 4 | Optimal | 8 | 12 | 60% | 40% |
| 5 | FIFO | 9 | 11 | 55% | 45% |
| 5 | LRU | 7 | 13 | 65% | 35% |
| 5 | Optimal | 7 | 13 | 65% | 35% |

Optimal performs best (or ties for best) for each frame count because it uses
future references; LRU matches it with 4 and 5 frames for this input. FIFO
performs worst in every case.

## Files and integration

The kernel files in this folder are the Assignment 9 RISC-V xv6 sources with
the Assignment 10 changes. `syscall.h`, `syscall.c`, `sysproc.c`, `defs.h`,
`user.h`, and `usys.pl` register the four new system calls. Copy these kernel
files into the corresponding xv6 kernel locations, place each `Q*/` test in
the xv6 user directory, and add `memtest`, `vttest`, `pagetest`, and
`pagereplace` to the `UPROGS` list in the xv6 Makefile.

The repository does not contain a complete xv6 checkout or Makefile, so the
kernel and syscall tests could not be booted here and no genuine xv6 terminal
screenshots are included. The page-replacement program was compiled with
warnings-as-errors against temporary host header shims and run; its results
match the table above. Run the four user programs in xv6 to capture the
submission screenshots required by the assignment.
