# Non-Dominated Points (Maxima of a Point Set)

A C++17 implementation and experimental study of the **maxima-of-a-point-set problem**: given N points in d-dimensional space, find all points that are not dominated by any other point (the "non-dominated" or "Pareto-optimal" points).

The project implements the divide-and-conquer **MAXIMA2** algorithm and its **FILTER** merge-step subroutine, following the classical treatment of the maxima problem in computational geometry (Preparata & Shamos, *Computational Geometry: An Introduction*).

---

## Project Overview

Given a set $S$ of $N$ points in $E^d$, a point $p$ **dominates** a point $q$ if $p$ is greater than or equal to $q$ in every coordinate, with strict inequality in at least one. A point is a **maximum** of $S$ if no other point in $S$ dominates it. The maxima problem asks for all such points.

This project investigates:

- Dominance relations between points
- Divide-and-conquer computation of maxima (`MAXIMA2`)
- The recursive `FILTER` merge routine and its 2D sweep base case (`FILTER2`)
- Empirical runtime behavior across dimension and input size
- Validation against hand-constructed test datasets

---

## Repository Structure

```text
non-dominated-points/
│
├── src/
│   ├── main.cpp            
│   ├── generate_data.cpp   # Random point generation
│   ├── dominance.cpp       # Dominance
│   ├── sorting.cpp         # Coordinate-based sorting helpers
│   ├── maxima2.cpp         # divide-and-conquer algorithm
│   ├── filter.cpp          # FILTER / FILTER2
│   ├── partition.cpp       # Equipartition of V for FILTER
│   ├── split_u.cpp         # Threshold-based split of U for FILTER
│   ├── threshold.cpp       # Threshold extraction from V1
│   ├── intersection.cpp    # intersection
│   └── print.cpp           # Console output helper
│
├── include/                # Corresponding headers
│
├── validation/
│   ├── validator.cpp/.hpp  # Loads hand-crafted test datasets
│   ├── validation_main.cpp # Runs MAXIMA2 against expected output
│   └── test_datasets/      # Test1.txt .. Test10.txt
│
├── plots/
│   ├── plot_results.py     # Generates runtime plots from benchmark CSV
│   └── figures/            # Output plots
│
├── results/
│   └── benchmark_results.csv
│
├── build/                  # CMake build directory
└── README.md
```

---

## Implemented Concepts

### 1. Dominance Relation

Point $p$ dominates point $q$ if:

```text
p[i] >= q[i]   for all coordinates i
p[i] >  q[i]   for at least one coordinate i
```

A point that is dominated by any other point can never be a maximum and is discarded.

### 2. Random Point Generation

`generateData(n, D)` produces `n` points with `D` integer coordinates, each drawn uniformly from `[1, 100]`, used to benchmark the algorithm across dimensions and input sizes.

### 3. MAXIMA2 — Divide-and-Conquer Maxima

1. Sort the point set by its last coordinate.
2. Split at the median (adjusted so that tied values on the split coordinate all land on the same side, preserving correctness).
3. Recursively compute the maxima of each half.
4. **Merge**: every maximum of the "upper" half survives automatically; a maximum of the "lower" half survives only if no point in the upper half's maxima dominates it on the remaining coordinates.

### 4. FILTER — Recursive Merge Step

`FILTER(U, V, d)` computes, for two point sets `U` and `V`, the subset of `U` not dominated by any point of `V`. It recurses by equipartitioning `V` and splitting `U` at the corresponding threshold, bottoming out at a linear-time 2D sweep (`FILTER2`) once only two coordinates remain relevant.

### 5. Validation

`validator` loads hand-built datasets from `validation/test_datasets/` (input points plus their expected maxima) and checks `MAXIMA2`'s output against them.

### 6. Benchmarking

`main.cpp` runs `MAXIMA2` over dimensions `2..10` and point counts `1,000..10,000`, averaging over 10 iterations per configuration, and writes timings to `results/benchmark_results.csv`. `plots/plot_results.py` then renders runtime curves into `plots/figures/`.

---

## Build Instructions

### Prerequisites

- C++17-capable compiler (GCC / Clang / MSVC)
- CMake ≥ 3.10
- Python 3 (for plotting benchmark results)

### Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

This produces two executables:

| Executable              | Purpose                                              |
|--------------------------|-------------------------------------------------------|
| `non_dominated_points`   | Runs the benchmark suite and generates runtime plots   |
| `validator`              | Runs `MAXIMA2` against the hand-crafted test datasets  |

### Run

```bash
./non_dominated_points   # benchmark + plot generation
./validator               # correctness validation against test datasets
```

---

## Experimental Results

Benchmark results are stored in `results/benchmark_results.csv` and visualized in `plots/figures/runtime.png`, showing execution time as a function of point count for each tested dimension.

<!-- TODO: Insert discussion of observed scaling behavior vs. theoretical bound -->

---

## Future Scope

Planned extensions to this work:

- **MPI (OpenMPI) implementation** — distribute the divide-and-conquer recursion of `MAXIMA2` / `FILTER` across multiple processes, with a focus on load-balanced merge steps.
- **CUDA implementation** — parallelize the dominance checks and the sweep/merge steps on the GPU for large point sets.
- **Performance evaluation**: measure **speedup** and **scale-up** of the parallel (MPI/CUDA) versions relative to the sequential baseline, across varying:
  - number of points $N$
  - dimension $d$
  - number of processes / threads / GPU cores
- Comparison of strong scaling vs. weak scaling behavior for both parallelization strategies.

---

## Author

**Arun Dinesh Jana**
Matrikel-Nr.: 2227229
Bergische Universität Wuppertal

---

## License

This project is intended for academic and research purposes.