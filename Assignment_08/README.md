# CS527: Computer Systems — Lab 8 Report
**Topic:** OpenMP — Arrays and Matrices  
**Language:** C++ (with OpenMP)  
**Compiler:** GCC / G++ with `-fopenmp -Wall -O2`  
**Test Environment:** Linux x86_64, Intel Core i5-10300H (4 Physical Cores, 8 Hardware Threads)

---

## 1. Overview & Objectives

The goal of Lab 8 is to explore parallel algorithms and data-parallel patterns in shared-memory multiprocessor architectures using OpenMP. The lab consists of five distinct tasks:
1. **Task 1: Array Search** — Parallel linear search with early termination and index retrieval.
2. **Task 2: Array Sorting** — Parallel Bubble Sort (Odd-Even Transposition Sort) and synchronization analysis.
3. **Task 3: Histogram Calculation** — Frequency count with race condition prevention (Atomic vs Thread-Local reduction).
4. **Task 4: Matrix Transpose** — 2D matrix transformation comparing outer-loop parallelization vs. `collapse(2)`.
5. **Task 5: 2D Convolution** — Applying a 3x3 averaging filter on image matrices, reproducing the exact 5x5 manual calculation and benchmarking large resolutions.

---

## 2. Compilation and Execution

A `Makefile` is provided in the workspace. All tasks compile cleanly under `-Wall -O2 -fopenmp -std=c++11`.

```bash
# To compile all tasks:
make all

# To run all tasks and print benchmark tables:
make run

# To run an individual task:
./task1_search
./task2_sort
./task3_histogram
./task4_transpose
./task5_convolution

# To clean compiled binaries:
make clean
```

---

## 3. Task-by-Task Implementation and Analysis

### Task 1: Array Search
- **Objective:** Search for an element in an array using OpenMP, determine if it exists, and retrieve its first index.
- **Parallel Strategy & OpenMP Constraints:**
  - In standard OpenMP `#pragma omp for` loops, `break` statements are prohibited because worksharing loops must have a single canonical entry and exit point.
  - To achieve early skipping without violating OpenMP loop specifications, threads monitor a shared variable `found_idx`. If another thread has already discovered the target at an index smaller than the current thread's iteration, `continue` is invoked.
  - When a thread finds a match, it updates `found_idx` inside a `#pragma omp critical` block, ensuring that only the smallest matching index is retained.
- **Test Scenarios:**
  - **Case A (Element Exists):** Placed unique target at index $N - 500$. Both serial and parallel implementations consistently locate and return the exact index.
  - **Case B (Element Does Not Exist):** Target absent from array. Both implementations scan the full array and return `-1`.
- **Timing & Speedup Table (Target at end of array, $N = 10^8$ integers, ~381.5 MB):**

| Threads | Execution Time (sec) | Speedup | Parallel Efficiency |
|:-------:|:--------------------:|:-------:|:-------------------:|
| Serial  | 0.048486 s           | 1.00x   | 1.00                |
| 1       | 0.105240 s           | 0.46x   | 0.46                |
| 2       | 0.054545 s           | 0.89x   | 0.44                |
| 4       | 0.047181 s           | 1.03x   | 0.26                |
| 8       | 0.033926 s           | 1.43x   | 0.18                |

- **Observation:** Linear search is memory-bandwidth bound. Once all threads access memory concurrently, memory bus saturation limits linear scaling. However, parallel search effectively reduces worst-case search latency across hardware threads.

---

### Task 2: Array Sorting (Odd-Even Transposition Bubble Sort)
- **Objective:** Implement parallel bubble sort and explain the synchronization required.
- **Why Standard Bubble Sort Cannot Be Parallelized:**
  - Standard bubble sort has strict loop-carried dependencies: comparison and swap of pair $(j, j+1)$ depends immediately on the state left by pair $(j-1, j)$. Performing standard bubble sort steps in parallel produces race conditions and incorrect ordering.
- **Odd-Even Transposition Sort (Parallel Bubble Sort):**
  - The algorithm splits the sorting process into $N$ alternating phases:
    1. **Even Phase:** Compares and swaps disjoint pairs $(0, 1), (2, 3), (4, 5), \dots$
    2. **Odd Phase:** Compares and swaps disjoint pairs $(1, 2), (3, 4), (5, 6), \dots$
  - Because pairs within a single phase are mutually disjoint, all swaps in that phase can execute concurrently without data races!
- **Synchronization Required:**
  - **Barrier Synchronization:** A strict synchronization barrier is required between the Even Phase and the Odd Phase. Phase $k+1$ depends on the values updated in Phase $k$.
  - In OpenMP, `#pragma omp for` introduces an **implicit barrier** at the end of each loop. Keeping the `#pragma omp parallel` team active outside the phase loop and placing `#pragma omp for` inside avoids thread recreation overhead while providing the required barrier synchronization.
- **Timing & Speedup Table (Worst-case reverse-sorted array):**

| Problem Size ($N$) | Serial Time | 1 Thread | 2 Threads (Speedup) | 4 Threads (Speedup) | 8 Threads (Speedup) |
|:------------------:|:-----------:|:--------:|:-------------------:|:-------------------:|:-------------------:|
| 5,000              | 0.0079 s    | 0.0085 s | 0.0056 s (1.41x)    | 0.0032 s (2.46x)    | 0.0064 s (1.22x)    |
| 10,000             | 0.0295 s    | 0.0336 s | 0.0227 s (1.43x)    | 0.0111 s (2.92x)    | 0.0112 s (2.89x)    |
| 20,000             | 0.1230 s    | 0.1405 s | 0.0748 s (1.64x)    | 0.0425 s (2.89x)    | 0.0777 s (1.58x)    |
| 30,000             | 0.2832 s    | 0.2870 s | 0.1595 s (1.77x)    | 0.0951 s (2.98x)    | 0.1569 s (1.80x)    |

- **Observation:** On 4 physical cores, the algorithm scales cleanly to nearly **3x speedup (74% parallel efficiency)**. At 8 threads, hyperthreading incurs barrier synchronization overhead across 30,000 phases, making 4 threads optimal.

---

### Task 3: Histogram Calculation
- **Objective:** Given an array of integers from 0 to 9, count frequency of each number using OpenMP while preventing race conditions.
- **Race Condition in Naive Parallelization:**
  - When threads execute `hist[a[i]]++` simultaneously, unsynchronized read-modify-write operations cause frequent lost updates.
  - **Empirical Demonstration:** On $N = 10,000,000$ elements, the naive parallel version returned an aggregate count of only **3,775,154** — losing **6,224,846 updates (~62.2% data loss!)**.
- **Prevention Techniques:**
  1. **Atomic (`#pragma omp atomic`):** Guarantees atomicity of the memory update. However, because all threads contend for the same 10 shared memory locations, severe hardware cache-line bouncing occurs.
  2. **Thread-Private Local Histograms (Best Practice):** Each thread allocates its own private 10-bin histogram (`vector<long> local_hist(10, 0)`). Threads count into their private bins with zero locking or contention. At the loop end, private counts are combined into the global histogram inside `#pragma omp critical`.
- **Timing & Speedup Table (Thread-Local Reduction, $N = 50,000,000$ integers, ~190.7 MB):**

| Threads | Execution Time (sec) | Speedup | Parallel Efficiency |
|:-------:|:--------------------:|:-------:|:-------------------:|
| Serial  | 0.032468 s           | 1.00x   | 1.00                |
| 1       | 0.028331 s           | 1.15x   | 1.15                |
| 2       | 0.015631 s           | 2.08x   | 1.04                |
| 4       | 0.010442 s           | 3.11x   | 0.78                |
| 8       | 0.016213 s           | 2.00x   | 0.25                |

- **Verification:** All 10 bins match serial counts exactly, and the sum of frequencies equals $N$.

---

### Task 4: Matrix Transpose
- **Objective:** Compute $B = A^T$ where $B[j][i] = A[i][j]$ using OpenMP.
- **Data Layout:** Row-major 1D flattened array: $B[j \times \text{rows} + i] = A[i \times \text{cols} + j]$.
- **Why Transpose Has No Race Conditions:** Each $(i, j)$ pair writes to a unique destination memory index $j \times \text{rows} + i$. Writes are completely disjoint.
- **Outer Loop vs. `collapse(2)` Comparison:**
  - For square matrices ($1024^2, 2048^2, 4096^2$), outer-loop parallelization already offers thousands of loop iterations, sufficient to saturate all cores.
  - For **tall/skinny matrices** with few rows (e.g. $16 \times 65,536$), the outer loop has only 16 iterations. `collapse(2)` combines the row and column loops into $16 \times 65,536 = 1,048,576$ fine-grained iterations.
- **Timing & Speedup Table for Tall Matrix ($16 \times 65,536$):**

| Threads | Outer Loop Time (sec) | Outer Speedup | Collapse(2) Time (sec) | Collapse(2) Speedup |
|:-------:|:---------------------:|:-------------:|:----------------------:|:-------------------:|
| Serial  | 0.012164 s            | 1.00x         | 0.012164 s             | 1.00x               |
| 1       | 0.013813 s            | 0.88x         | 0.014693 s             | 0.83x               |
| 2       | 0.012394 s            | 0.98x         | 0.014574 s             | 0.83x               |
| 4       | 0.008221 s            | 1.48x         | 0.009590 s             | 1.27x               |
| 8       | 0.010790 s            | 1.13x         | **0.002589 s**         | **4.70x**           |

- **Observation:** On the tall matrix, `collapse(2)` with 8 threads achieved a **4.70x speedup**, significantly outperforming the outer-loop parallelization (**1.13x**) due to improved work distribution.

---

### Task 5: 2D Convolution (3x3 Averaging Kernel)
- **Objective:** Apply a 3x3 averaging filter ($K = \frac{1}{9} \mathbf{1}_{3\times 3}$) to an image matrix.
- **Reproduction of Lab Manual 5x5 Example:**
  - Input Image (5x5):
    $$\begin{bmatrix}
    10 & 20 & 30 & 40 & 50 \\
    60 & 70 & 80 & 90 & 100 \\
    110 & 120 & 130 & 140 & 150 \\
    160 & 170 & 180 & 190 & 200 \\
    210 & 220 & 230 & 240 & 250
    \end{bmatrix}$$
  - First $3\times 3$ window centered at $(1, 1)$:
    $$O[0][0] = \frac{10+20+30+60+70+80+110+120+130}{9} = \frac{630}{9} = 70.0$$
  - Resulting $3\times 3$ Valid Output Matrix:
    $$\begin{bmatrix}
    70.0 & 80.0 & 90.0 \\
    120.0 & 130.0 & 140.0 \\
    170.0 & 180.0 & 190.0
    \end{bmatrix}$$
  - **Verification:** The implementation matches the manual's example and expected output with $100\%$ precision.
- **Parallel Scaling on Realistic Image Resolutions ($512^2, 1024^2, 2048^2, 4096^2$):**

| Resolution ($H \times W$) | Serial Time | 1 Thread | 2 Threads (Speedup) | 4 Threads (Speedup) |
|:-------------------------:|:-----------:|:--------:|:-------------------:|:-------------------:|
| $512 \times 512$          | 0.002316 s  | 0.000972 s| 0.000624 s (3.71x)  | 0.000412 s (5.62x)  |
| $1024 \times 1024$        | 0.003900 s  | 0.005117 s| 0.002357 s (1.66x)  | 0.001745 s (2.23x)  |
| $2048 \times 2048$        | 0.015985 s  | 0.015041 s| 0.007803 s (2.05x)  | 0.008778 s (1.82x)  |
| $4096 \times 4096$        | 0.067669 s  | 0.072996 s| 0.043765 s (1.55x)  | 0.041864 s (1.62x)  |

- **Observation:** 2D convolution is embarrassingly parallel because image reading is strictly read-only and each thread writes to distinct output pixels.

---

## 4. Submission Checklist Verification

- [x] **Clean Compilation:** All five tasks compile cleanly with `-fopenmp -Wall -O2` with zero warnings.
- [x] **Correctness Verification:** Every parallel routine includes verification against the serial baseline.
- [x] **Timing Tables:** Execution time measured using `omp_get_wtime()` across thread counts 1, 2, 4, 8 for multiple problem sizes.
- [x] **Speedup & Efficiency:** Speedup ($T_{\text{serial}} / T_{\text{parallel}}$) and parallel efficiency computed and formatted cleanly.
- [x] **Synchronization & Race Condition Analysis:** Explicit documentation and explanations provided for sorting synchronization and histogram race conditions.

