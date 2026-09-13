# Lab6


---

### Question 3: Deadlock Prevention via Resource Ordering

#### Executive Summary

Deadlock requires four simultaneous Coffman conditions: Mutual Exclusion, Hold and Wait, No Preemption, and **Circular Wait**. This experiment demonstrates how imposing a global resource ordering hierarchy eliminates Circular Wait.

#### Analysis of Scenarios

* **Unordered Allocation (`$ resourceorder`):** Process A acquires `Lock1` and requests `Lock2`. Simultaneously, Process B acquires `Lock2` and requests `Lock1`. This creates a circular dependency graph ($P_A \to P_B \to P_A$), resulting in permanent execution hang (deadlock).
* **Hierarchical Allocation (`$ resourceorder fix`):** A strict global ordering rule Lock1 Lock2 is enforced. Both processes acquire resources in ascending order.

#### Conclusion

By enforcing $F(R_i) < F(R_j)$, a process holding $R_i$ can only request $R_j$ if $j > i$. This structural constraint renders directed cycles impossible in the Wait-For Graph, structurally eliminating Circular Wait and guaranteeing deadlock-free execution.

---

### Question 4: Combined Synchronization & Deadlock Avoidance

#### Selected Strategy

**Hierarchical Resource Ordering Strategy** paired with atomic user-space spinlocks.

#### Strategy Justification

In a multi-process environment sharing limited resource types ($\text{Printer} = 0$, $\text{Scanner} = 1$, $\text{Disk} = 2$), dynamic lock requests risk circular wait conditions across concurrent processes.

To resolve this:

1. **Resource Identification:** Each resource type is mapped to a unique integer ID ($0, 1, 2$).
2. **Ascending Request Rule:** Each process sorted its required pair of resources in strictly ascending order ($R_{\text{first}} < R_{\text{second}}$) before issuing acquisition requests.

#### Results

Across multi-cycle executions of all 5 processes ($P_0$ through $P_4$), the Wait-For Graph remained strictly acyclic at all times. Every process successfully acquired, used, and released its resource pairs without experiencing permanent blocking or violating available instance bounds, exiting cleanly with zero deadlock.
