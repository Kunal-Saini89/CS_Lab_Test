# CS527: Computer Systems — Lab Assignment 7
## Introduction to OpenMP: Arrays and Matrices

**Student Name:** Kunal  
**Course:** CS527 – Computer Systems  
**Environment:** Linux x86_64, GCC 16.1.1 (`-fopenmp -O2 -Wall`), Intel Core i5-10300H (4 Cores / 8 Threads @ 2.50GHz, 8MB L3 Cache, 8GB DDR4 RAM)  
**Thread Management:** Threads set using `omp_set_num_threads()` in source code and validated with `OMP_NUM_THREADS`. Wall-clock timing measured via `omp_get_wtime()`.

---

## Quick Start & Compilation

A `Makefile` is provided to compile all tasks cleanly with `-fopenmp -O2 -Wall`:

```bash
# Build all five tasks
make

# Run individual task benchmarks
./task1
./task2
./task3
./task4
./task5

# Clean binaries
make clean
```

---

## Task 1 — Vector Addition

### 1. Implementation Summary
- Problem: Compute $c[i] = a[i] + b[i]$ for double-precision floating point arrays of sizes $N = 10^5, 10^6, 10^7, 10^8$.
- Serial: standard single loop over index $i \in [0, N-1]$.
- Parallel: `#pragma omp parallel for` distributing loop iterations among threads with uniform static chunking.
- Verification: Element-wise comparison between serial and parallel results (`c_serial[i] == c_parallel[i]` across all $N$ elements).
- Note: Pre-faulting/touching allocated memory was performed before timing runs so page-fault latencies do not distort the computational timing.

### 2. Experimental Timing Table
All times are reported in seconds. Speedup is $S = T_{\text{serial}} / T_{\text{parallel}}$, Efficiency is $E = S / \text{threads}$.

| Array Size ($N$) | Vector Size (MB) | Serial Time (s) | Threads = 1 | Threads = 2 | Threads = 4 | Threads = 8 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **$10^5$** | 0.8 MB | 0.000175 s | 0.000121 s (1.45x) | 0.000054 s (3.27x) | 0.000037 s (4.80x) | 0.002219 s (0.08x) |
| **$10^6$** | 7.6 MB | 0.002738 s | 0.003137 s (0.87x) | 0.003640 s (0.75x) | 0.003407 s (0.80x) | 0.007076 s (0.39x) |
| **$10^7$** | 76.3 MB | 0.026899 s | 0.024259 s (1.11x) | 0.029129 s (0.92x) | 0.024961 s (1.08x) | 0.032989 s (0.82x) |
| **$10^8$** | 762.9 MB | 0.340236 s | 0.316806 s (1.07x) | 0.272039 s (1.25x) | 0.244136 s (1.39x) | 0.246014 s (1.38x) |

**Verification Result:** `SUCCESS` (Serial and Parallel match exactly across all sizes).

#### Detailed Speedup & Efficiency for Largest $N = 10^8$:
| Threads | Execution Time (s) | Speedup ($T_{\text{serial}} / T_{\text{par}}$) | Parallel Efficiency ($S / P$) |
| :---: | :---: | :---: | :---: |
| 1 | 0.316806 | 1.07x | 1.07 |
| 2 | 0.272039 | 1.25x | 0.63 |
| 4 | 0.244136 | 1.39x | 0.35 |
| 8 | 0.246014 | 1.38x | 0.17 |

### 3. Answers to Task 1 Questions
- **At what array size does parallelization actually start paying off? Why is it not beneficial for small $N$?**  
  Parallelization only begins to pay off when the total computational work significantly outweighs the overhead of thread fork/join, barrier synchronization, and work scheduling in the OpenMP runtime. For small $N$ ($10^5$ to $10^6$), launching and managing multiple threads costs more time than the actual vectorized loop execution, which finishes in less than a millisecond. Furthermore, at 8 threads on small arrays, thread scheduling and context switching overhead cause severe degradation (speedup drops to 0.08x).
- **Does speedup keep increasing linearly with thread count? Why or why not?**  
  No, speedup does not scale linearly and plateaus around 1.39x. Vector addition performs only 1 floating-point addition for every 2 reads (`a[i]`, `b[i]`) and 1 write (`c[i]`) of 8-byte doubles (an arithmetic intensity of only $1/24 \approx 0.042$ FLOP/byte). For $N = 10^8$, each vector is ~763 MB, totaling ~2.3 GB of memory traffic, which completely exceeds the 8 MB L3 cache and saturates the main memory (DRAM) bandwidth. Once the DRAM bus bandwidth is saturated, adding more CPU threads cannot fetch data any faster, so execution time is strictly memory-bandwidth bound rather than compute-bound.

---

## Task 2 — Sum of Array Elements

### 1. Implementation Summary
- Problem: Compute $\sum_{i=0}^{N-1} a[i]$ over an array of size $N = 10^7, 10^8$.
- Naive Parallel (BROKEN): Multiple threads concurrently updating a shared scalar `sum += a[i]` without synchronization.
- Reduction: Correct parallelization using `#pragma omp parallel for reduction(+:sum)`.
- Critical Section: `#pragma omp critical` enclosing `sum += a[i]` to demonstrate the locking penalty.

### 2. Experimental Verification & Observations

#### Demonstration of Data Race on Naive Parallel Sum ($N = 10^7$, 4 threads, Expected Sum = $10^7$):
- **Run 1:** Computed Sum = `2,500,000` (Missing: 7,500,000)
- **Run 2:** Computed Sum = `2,500,000` (Missing: 7,500,000)
- **Run 3:** Computed Sum = `2,500,000` (Missing: 7,500,000)
- **Run 4:** Computed Sum = `2,500,000` (Missing: 7,500,000)
- **Run 5:** Computed Sum = `2,500,000` (Missing: 7,500,000)

**Explanation:** In the naive version, `sum += a[i]` compiles into three machine operations: `LOAD sum into register`, `ADD a[i]`, and `STORE register back into sum`. Because all 4 threads execute concurrently on shared memory without locks, they constantly read stale values of `sum` and overwrite each other's additions, discarding ~75% of the updates.

#### Reduction Verification:
- Every single run of `sum_reduction` produced `10,000,000` matching the serial baseline (`PASS`).

### 3. Timing Benchmark (Reduction across Threads)

| Array Size ($N$) | Serial Time (s) | 1 Thread (s) | 2 Threads (s) | 4 Threads (s) | 8 Threads (s) | Max Speedup |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **$10^7$** (76 MB) | 0.006272 | 0.011715 (0.54x) | 0.008798 (0.71x) | 0.006642 (0.94x) | 0.006964 (0.90x) | 0.94x |
| **$10^8$** (763 MB)| 0.078115 | 0.076913 (1.02x) | 0.063903 (1.22x) | 0.056628 (1.38x) | 0.060499 (1.29x) | 1.38x |

#### Critical Section vs. Reduction Comparison ($N = 10^7$, 4 threads):
- **`sum_reduction` Execution Time:** `0.006258 seconds`
- **`sum_critical` Execution Time:** `0.816314 seconds`
- **Slowdown Factor:** Critical section is **130.5x slower** than reduction!

### 4. Answers to Task 2 Questions
- **Why does the naive version (step 1) give wrong/inconsistent answers? Explain in terms of a data race on the shared variable `sum`.**  
  `sum += a[i]` is not an atomic operation. It consists of reading `sum` from cache/RAM into a CPU register, adding `a[i]` to that register, and writing the result back to memory. When multiple threads do this simultaneously on a shared variable, thread interleaving causes classic read-after-write conflicts: Thread B reads `sum` before Thread A finishes writing its updated value, and then Thread B writes its value, completely clobbering Thread A's addition.
- **Would using `#pragma omp critical` around `sum += a[i]` fix correctness? What would you expect to happen to performance if you did that instead of using `reduction`?**  
  Yes, `#pragma omp critical` forces mutual exclusion so only one thread can execute `sum += a[i]` at any instant, eliminating the race condition and producing the correct sum. However, performance degrades catastrophically: each iteration incurs the hardware overhead of acquiring and releasing a lock and invalidating cache lines across cores. With $10^7$ iterations, serialization and lock contention caused the critical section to be **130.5x slower** than the reduction clause. In contrast, `reduction` gives each thread a private accumulator in a local register/cache without any locks, combining them only once at the end in $O(\text{num\_threads})$ time.

---

## Task 3 — Maximum and Minimum of an Array

### 1. Implementation Summary
- Problem: Find both the extreme value (max/min) and its 0-indexed position (`max_idx`, `min_idx`) in an array of $N = 10^7, 10^8$ integers.
- OpenMP Value-Only Reduction: `#pragma omp parallel for reduction(max:max_val)` and `reduction(min:min_val)`.
- Index Tracking Approaches:
  1. **Critical Section Approach:** A shared `(best_val, best_idx)` pair updated inside `#pragma omp critical`.
  2. **Manual Chunk Reduction Approach:** Each thread finds its local `(my_val, my_idx)` within its partition range $[(tid \cdot N)/P, ((tid+1)\cdot N)/P)$, and stores it in thread-local arrays. A final serial pass combines the $P$ thread candidates.

### 2. Experimental Timing Table
Manual Chunk Reduction timings for finding both Max and Min (values and indices):

| Array Size ($N$) | Serial Baseline (s) | 1 Thread (s) | 2 Threads (s) | 4 Threads (s) | 8 Threads (s) | Max Speedup |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **$10^7$** (38 MB) | 0.028517 | 0.019911 (1.43x) | 0.015014 (1.90x) | 0.011205 (2.55x) | 0.016681 (1.71x) | **2.55x** |
| **$10^8$** (381 MB)| 0.198166 | 0.201965 (0.98x) | 0.107170 (1.85x) | 0.069060 (2.87x) | 0.106630 (1.86x) | **2.87x** |

**Correctness Verification:** Both `reduction(max/min)` and the manual chunk reduction produced the exact target values (`2000000000`, `-2000000000`) and indices (`3333333`, `6666666` for $10^7$; `33333333`, `66666666` for $10^8$), matching the serial baseline (`PASS`).

#### Critical Section vs. Manual Chunk Reduction (4 Threads):
- For $N = 10^7$: Manual = `0.009196 s`, Critical = `0.782424 s` $\rightarrow$ Critical is **85.1x slower**.
- For $N = 10^8$: Manual = `0.045102 s`, Critical = `8.242207 s` $\rightarrow$ Critical is **182.7x slower**.

### 3. Answers to Task 3 Questions
- **Why can't you directly write `reduction(max: best_idx)` to get the index of the maximum?**  
  The `reduction(max: ...)` operator applies the mathematical $\max$ function on the reduced variable itself. If you write `reduction(max: best_idx)`, OpenMP will simply find the maximum *index number* (which is always $N-1$), completely ignoring the array values stored at those indices. Standard OpenMP C/C++ reduction operators do not support custom compound reduction tuples like `argmax(val, idx)` natively without complex user-defined reductions (which were only standardized in OpenMP 4.0+ and still require custom combiner functions).
- **Compare the performance of the `critical`-section approach vs. the manual per-thread reduction approach for finding `(max_val, max_idx)`. Which is faster, and why?**  
  The **manual per-thread reduction approach is dramatically faster** (by 85x to 182x). In the manual approach, each thread works entirely independently on its contiguous chunk in private L1/L2 cache and CPU registers with zero cross-core communication, locks, or synchronization. Synchronization occurs only once at the end when combining the $P$ results (where $P \le 8$). In contrast, the critical-section approach forces all threads to serialize and contend on a central lock, repeatedly stalling execution pipelines and causing high bus traffic from cache coherence invalidations.

---

## Task 4 — Matrix Addition

### 1. Implementation Summary
- Problem: $C[i][j] = A[i][j] + B[i][j]$ for $N \times N$ matrices.
- Storage: Matrices stored as contiguous flattened 1D arrays of size $N^2$ with row-major indexing `index = i * N + j` (avoids heap fragmentation and guarantees cache-friendly contiguous access).
- Evaluated Variants:
  1. Serial nested loops.
  2. Outer loop parallelization: `#pragma omp parallel for` on row loop $i$.
  3. Collapsed loops: `#pragma omp parallel for collapse(2)` combining loops $i$ and $j$.
- Sizes:
  - Small matrix: $N = 8$ (64 elements; measured over $10^6$ repetitions to capture stable execution metrics).
  - Large matrix: $N = 4096$ (16.78 million elements, ~128 MB per matrix).

### 2. Experimental Timing Table

#### Small Matrix ($N = 8$, repeated $10^6$ times, Serial Time = 0.029482 s):
| Threads | Outer Loop Time (s) | Collapse(2) Time (s) | Outer Speedup | Collapse Speedup |
| :---: | :---: | :---: | :---: | :---: |
| 1 | 0.375175 | 0.406388 | 0.08x | 0.07x |
| 2 | 0.723945 | 0.802915 | 0.04x | 0.04x |
| 4 | 1.033902 | 1.073728 | 0.03x | 0.03x |
| 8 | 5.126955 | 2.211688 | 0.01x | 0.01x |

*Notice:* At 8 threads, `collapse(2)` was **2.3x faster than outer loop** (2.21s vs 5.12s) because it expanded the loop space from 8 to 64 iterations, distributing work evenly across threads.

#### Large Matrix ($N = 4096$, single pass, Serial Time = 0.045896 s):
| Threads | Outer Loop Time (s) | Collapse(2) Time (s) | Outer Speedup | Collapse Speedup |
| :---: | :---: | :---: | :---: | :---: |
| 1 | 0.040946 | 0.045260 | 1.12x | 1.01x |
| 2 | 0.039854 | 0.045855 | 1.15x | 1.00x |
| 4 | 0.042541 | 0.048759 | 1.08x | 0.94x |
| 8 | 0.053361 | 0.068566 | 0.86x | 0.67x |

**Verification Result:** Both `mat_add_outer` and `mat_add_collapse` matched the serial matrix across all elements (`PASS`).

### 3. Answers to Task 4 Questions
- **For the small-$N$ case, does `collapse(2)` help compared to parallelizing only the outer loop? Explain why, in terms of how many independent iterations are available to distribute across threads in each version.**  
  Yes, for $N = 8$ with 8 threads, `collapse(2)` significantly outperforms parallelizing the outer loop alone (2.21 s vs 5.12 s). In the outer-loop-only version, there are only $N = 8$ iterations to distribute among 8 threads, meaning each thread gets exactly 1 row. Any slight load imbalance or thread launch skew causes severe idle time. In contrast, `collapse(2)` merges both loops into a single iteration space of $N \times N = 64$ iterations. Distributing 64 chunks among 8 threads provides 8 chunks per thread, allowing the OpenMP scheduler to balance work much more smoothly. (Overall performance is still lower than serial because 64 additions per call cannot amortize thread barrier overhead, but `collapse(2)` mitigates the worst thread starvation).
- **For the large-$N$ case, does `collapse(2)` still make a measurable difference, or is parallelizing the outer loop alone already enough to keep all threads busy?**  
  For $N = 4096$, parallelizing the outer loop alone is **already more than enough** to keep all threads fully saturated. With $N = 4096$ and 8 threads, each thread is assigned $4096 / 8 = 512$ complete rows, which translates to $512 \times 4096 \approx 2.1$ million additions per thread. Collapsing the loops to $16.78$ million iterations provides no extra concurrency benefit, and actually introduces slight runtime index calculation overhead ($i = k / N, j = k \% N$). Hence, outer-loop parallelization performs slightly better than `collapse(2)` for large matrices.

---

## Task 5 — Matrix-Matrix Multiplication (Naive)

### 1. Implementation Summary
- Problem: $C = A \times B$ where $C[i][j] = \sum_{k=0}^{N-1} A[i][k] \cdot B[k][j]$ for $N = 256, 512, 1024$.
- Serial: Classic triple loop ($i, j, k$) accumulating into a local register scalar `sum` before writing $C[i \cdot N + j]$.
- Parallel Outer Loop: `#pragma omp parallel for` on outer loop $i$.
- Parallel Collapse: `#pragma omp parallel for collapse(2)` on loops $i$ and $j$.
- **Bonus Optimization:** Loop interchange to order $(i, k, j)$. In $(i, k, j)$, matrix $A[i][k]$ is loop-invariant with respect to inner loop $j$, and both $B[k][j]$ and $C[i][j]$ are accessed with unit stride (row-major). This produces optimal spatial cache locality and enables compiler SIMD vectorization.
- Verification: Tested with tolerance $\epsilon = 10^{-9}$ against the serial baseline (`verify_matmul`).

### 2. Experimental Timing Tables

#### Matrix Multiplication ($N = 256$, Serial Time = 0.0392 s):
| Threads | Outer Time (s) | Outer Speedup | Outer Eff. | Collapse Time (s) | Collapse Speedup | Collapse Eff. |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 1 | 0.0418 | 0.94x | 0.94 | 0.0417 | 0.94x | 0.94 |
| 2 | 0.0224 | 1.75x | 0.88 | 0.0208 | 1.88x | 0.94 |
| 4 | 0.0127 | 3.08x | 0.77 | 0.0229 | 1.72x | 0.43 |
| 8 | 0.0379 | 1.03x | 0.13 | 0.0493 | 0.80x | 0.10 |

#### Matrix Multiplication ($N = 512$, Serial Time = 0.3376 s):
| Threads | Outer Time (s) | Outer Speedup | Outer Eff. | Collapse Time (s) | Collapse Speedup | Collapse Eff. |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 1 | 0.3214 | 1.05x | 1.05 | 0.3434 | 0.98x | 0.98 |
| 2 | 0.1611 | 2.10x | 1.05 | 0.1566 | 2.16x | 1.08 |
| 4 | 0.0847 | 3.98x | 1.00 | 0.0872 | 3.87x | 0.97 |
| 8 | 0.0830 | 4.07x | 0.51 | 0.0751 | 4.50x | 0.56 |

#### Matrix Multiplication ($N = 1024$, Serial Time = 8.3619 s):
| Threads | Outer Time (s) | Outer Speedup | Outer Eff. | Collapse Time (s) | Collapse Speedup | Collapse Eff. |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 1 | 7.4903 | 1.12x | 1.12 | 8.1620 | 1.02x | 1.02 |
| 2 | 3.9385 | 2.12x | 1.06 | 3.4490 | 2.42x | 1.21 |
| 4 | 3.5070 | 2.38x | 0.60 | 3.0797 | 2.72x | 0.68 |
| 8 | 3.7890 | 2.21x | 0.28 | 3.0635 | 2.73x | 0.34 |

---

### 3. Bonus: Loop Order Reordering $(i, k, j)$ Results
Reordering loops to $(i, k, j)$ transforms the memory access of $B[k][j]$ and $C[i][j]$ from strided column-wise accesses into unit-stride row-wise accesses.

For $N = 1024$:
- **Serial $(i, j, k)$ Time:** `8.3619 seconds`
- **Serial $(i, k, j)$ Time:** `1.1575 seconds` (**7.22x faster** without any parallelism!)
- **Parallel $(i, k, j)$ Timings:**
  - 1 Thread: `1.2914 s` (6.48x vs serial $i,j,k$)
  - 2 Threads: `0.6118 s` (13.67x vs serial $i,j,k$)
  - 4 Threads: `0.2975 s` (**28.11x** vs serial $i,j,k$)
  - 8 Threads: `0.2350 s` (**35.58x** vs serial $i,j,k$)

### 4. Answers to Task 5 Questions
- **Why is it safe to parallelize the `i` loop directly (no race condition), even though the innermost `k` loop is doing a summation into `C[i][j]`?**  
  Every thread is assigned a distinct, non-overlapping subset of row indices $i$. For a given $i$, thread $T$ computes elements $C[i][0 \dots N-1]$. Since no two threads ever share the same row index $i$, no two threads ever write to or read from the same memory location $C[i][j]$. Accumulation in the $k$ loop is strictly private to the thread handling row $i$. Because data dependencies exist only across $k$ for a fixed $(i, j)$, parallelizing the independent outer loop $i$ is completely race-free.
- **What happens to correctness if you try to parallelize the `k` loop directly with a plain `parallel for` (no reduction)? Explain the race condition that would occur on `C[i][j]`.**  
  If the $k$ loop is parallelized directly with `#pragma omp parallel for`, all threads would concurrently execute iterations of $k$ for the *exact same* pair of $(i, j)$. In each iteration, threads would attempt to execute `C[i][j] += A[i][k] * B[k][j]`. Because $C[i][j]$ is a single shared memory address, concurrent read-modify-write operations without synchronization cause severe data races. Threads would overwrite each other's partial sums, yielding wildly incorrect and non-deterministic results.
- **Compare `parallel for` on `i` alone vs. `collapse(2)` on `(i, j)`. For which values of `N` (relative to your thread count) would you expect `collapse(2)` to matter most?**  
  `collapse(2)` matters most when $N$ is small relative to the number of threads $P$ (for instance, when $N \le 2P$). If $N = 8$ and $P = 8$, parallelizing only $i$ gives each thread exactly 1 row; if $P = 16$, half the threads would be idle. Collapsing $(i, j)$ expands the iteration space from $N$ to $N^2$ (from 8 to 64), providing ample chunks to distribute evenly across all cores. However, once $N$ is sufficiently large (e.g. $N \ge 256$), $N$ already provides dozens or hundreds of rows per thread, so the benefits of `collapse(2)` diminish and outer loop parallelization suffices.

---

## Submission Checklist Verification
- [x] Source files for all five tasks (`task1_vecadd.c`, `task2_sum.c`, `task3_minmax.c`, `task4_matadd.c`, `task5_matmul.c`), compiling cleanly with `-fopenmp -Wall -O2`.
- [x] Makefile included for single-command builds.
- [x] Correctness verification implemented and confirmed (`PASS`) for all parallel implementations against serial baselines.
- [x] Complete timing tables for all specified input sizes and thread counts ($1, 2, 4, 8$).
- [x] Speedup and parallel efficiency calculations reported.
- [x] Written answers to every conceptual and performance question.

