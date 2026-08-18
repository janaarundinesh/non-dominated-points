# Non-Dominated Points (Maxima of a Point Set)

A C++17 implementation and experimental study of the **maxima-of-a-point-set problem**: given N points in d-dimensional space, find all points that are not dominated by any other point (the "non-dominated" or "Pareto-optimal" points).

The project implements the divide-and-conquer **MAXIMA2** algorithm and its **FILTER** merge-step subroutine, following the classical treatment of the maxima problem in computational geometry (Preparata & Shamos, *Computational Geometry: An Introduction*), and benchmarks it against a brute-force baseline.

---

## Project Overview

Given a set $S$ of $N$ points in $E^d$, a point $p$ **dominates** a point $q$ if $p$ is greater than or equal to $q$ in every coordinate, with strict inequality in at least one. A point is a **maximum** of $S$ if no other point in $S$ dominates it. The maxima problem asks for all such points.

This project investigates:

- Dominance relations between points
- Divide-and-conquer computation of maxima (`MAXIMA2`)
- The recursive `FILTER` merge routine and its 2D sweep base case (`FILTER2`)
- A brute-force $O(N^2 d)$ baseline for correctness and performance comparison
- Empirical runtime behavior across dimension and input size
- Validation against hand-constructed test datasets

---

## Repository Structure

```text
non-dominated-points/
│
├── src/
│   ├── main.cpp               # Benchmark driver for MAXIMA2
│   ├── main_bruteforce.cpp    # Benchmark driver for the brute-force baseline
│   ├── main_gen_data.cpp      # CLI entry point for random dataset generation
│   ├── generate_data.cpp      # Random point generation
│   ├── dominance.cpp          # Dominance relation
│   ├── sorting.cpp            # Coordinate-based sorting helpers
│   ├── maxima2.cpp            # MAXIMA2 divide-and-conquer algorithm
│   ├── brute_force_maxima.cpp # Brute-force O(N^2) maxima
│   ├── filter.cpp             # FILTER / FILTER2
│   ├── partition.cpp          # Equipartition of V for FILTER
│   ├── split_u.cpp            # Threshold-based split of U for FILTER
│   ├── threshold.cpp          # Threshold extraction from V1
│   ├── intersection.cpp       # Set intersection helper
│   ├── benchmark.cpp          # Timing / CSV output helpers
│   └── print.cpp              # Console output helper
│
├── include/                   # Corresponding headers
│
├── DataSets/                  # Generated benchmark inputs, grouped by dimension (5D-10D)
│
├── validation/
│   ├── validator.cpp/.hpp     # Loads hand-crafted test datasets
│   ├── validation_main.cpp    # Runs MAXIMA2 against expected output (13 test cases)
│   └── test_datasets/         # Test1.txt .. Test13.txt
│
├── plots/
│   ├── plot_results.py        # Generates runtime plots from benchmark CSVs
│   └── figures/                # Output plots
│
├── results/
│   ├── maxima_benchmark.csv
│   └── brute_force_benchmark.csv
│
├── figures/                    # Report figures (e.g. runtime.png)
├── Thesis_Report.pdf
├── build/                      # CMake build directory
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

`generateData(n, D)` produces `n` points with `D` integer coordinates, each drawn uniformly from `[1, 100]`, used to benchmark the algorithms across dimensions and input sizes.

### 3. Brute-Force Maxima

`BruteForceMaxima(points, d)` checks every point against every other point directly ($O(N^2 d)$) and keeps the ones that are never dominated. It's the correctness and performance baseline that `MAXIMA2` is measured against, and it's also what `MAXIMA2` falls back to when a recursive split can't separate the points (e.g. all points tie on the split coordinate).

### 4. MAXIMA2 — Divide-and-Conquer Maxima

1. Sort the point set by its last coordinate.
2. Split at the median (adjusted so that tied values on the split coordinate all land on the same side, preserving correctness). If the split can't separate the set at all, fall back to `BruteForceMaxima`.
3. Recursively compute the maxima of each half, giving `M1` (lower half) and `M2` (upper half).
4. **Merge**: every point of `M2` survives automatically, since it was sorted to dominate on the split coordinate. A point of `M1` survives only if no point of `M2` dominates it on the remaining `d-1` coordinates — this check is delegated to `FILTER`.

### 5. FILTER — Recursive Merge Step

`FILTER(U, V, d)` computes, for two point sets `U` and `V`, the subset of `U` not dominated by any point of `V`. It recurses by equipartitioning `V` and splitting `U` at the corresponding threshold, recursing three ways (`U2` vs `V2`, `U1` vs `V1`, and `U1` vs `V2` with one fewer dimension), and combining the results. It bottoms out at a linear-time 2D sweep (`FILTER2`) once only two coordinates remain relevant, plus a few cheap base cases (`U` empty, `V` empty, `|V| = 1`, `d = 1`).

### 6. Validation

`validator` loads hand-built datasets from `validation/test_datasets/` (input points plus their expected maxima) and checks `MAXIMA2`'s output against all 13 of them.

### 7. Benchmarking

`non_dominated_points` runs `MAXIMA2` and `brute_force` runs `BruteForceMaxima` over the generated datasets in `DataSets/`, averaging over multiple iterations per configuration, and writes median timings to `results/maxima_benchmark.csv` and `results/brute_force_benchmark.csv` respectively. `plots/plot_results.py` then renders runtime curves into `plots/figures/`.

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

This produces four executables:

| Executable              | Purpose                                                        |
|--------------------------|------------------------------------------------------------------|
| `non_dominated_points`   | Runs the `MAXIMA2` benchmark suite over the generated datasets   |
| `brute_force`            | Runs the brute-force baseline benchmark over the same datasets   |
| `generate_data`          | Generates random point datasets into `DataSets/`                 |
| `validator`              | Runs `MAXIMA2` against the hand-crafted test datasets             |

### Run

```bash
./generate_data            # (re)generate benchmark datasets
./non_dominated_points      # MAXIMA2 benchmark
./brute_force                # brute-force benchmark
./validator                   # correctness validation against test datasets
python3 ../plots/plot_results.py   # render runtime plots
```

---

## Experimental Results

Benchmark results are stored in `results/maxima_benchmark.csv` and `results/brute_force_benchmark.csv`, and visualized in `plots/figures/runtime.png`, showing execution time as a function of point count for each tested dimension (2 through 10).

<!-- TODO: Insert discussion of observed scaling behavior vs. theoretical bound -->

---

## Roadmap

The current codebase is a sequential baseline. Planned next steps:

- **Parallel brute-force implementation** — parallelize the pairwise dominance checks (embarrassingly parallel across the outer loop), as a first, simple speedup target before tackling the divide-and-conquer version.
- **Parallel MAXIMA2 / FILTER implementation** — parallelize the divide-and-conquer recursion itself: the two recursive `MAXIMA2` calls on `S1`/`S2` are independent and can run concurrently, as can `FILTER`'s three-way recursive split (`U2` vs `V2`, `U1` vs `V1`, `U1` vs `V2`), with attention to load balancing since the three branches aren't generally the same size.
  - **MPI (OpenMPI)** — distribute the recursion across multiple processes.
  - **CUDA** — parallelize the dominance checks and the sweep/merge steps on the GPU for large point sets.
  - **Multithreading (e.g. `std::thread` / OpenMP)** as a lighter-weight intermediate step before MPI/CUDA, if useful for shared-memory experiments.
- **Speedup and scale-up evaluation** — benchmark both parallel versions against their sequential counterparts across:
  - number of points $N$
  - dimension $d$
  - number of processes / threads / GPU cores
  - strong scaling vs. weak scaling behavior
- **Central hypothesis to test**: that the parallel divide-and-conquer version outperforms the parallel brute-force version at scale — i.e. that algorithmic efficiency (sub-quadratic sequential work) compounds with parallelism rather than being made irrelevant by it. This means benchmarking both baselines *and* both parallel versions side by side, not just the two sequential or two parallel versions in isolation, so the comparison isolates the effect of the algorithm from the effect of parallelization.

---

## Author

**Arun Dinesh Jana**
Matrikel-Nr.: 2227229
Bergische Universität Wuppertal

---

## License

This project is intended for academic and research purposes.
