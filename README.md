# Non-Dominated Points (Maxima of a Point Set)

A C++17 implementation and experimental study of the **maxima-of-a-point-set problem**: given N points in d-dimensional space, find all points that are not dominated by any other point (the "non-dominated" or "Pareto-optimal" points).

The project implements the divide-and-conquer **MAXIMA2** algorithm and its **FILTER** merge-step subroutine, following the classical treatment of the maxima problem in computational geometry (Preparata & Shamos, *Computational Geometry: An Introduction*), and benchmarks it against a brute-force baseline. MPI prototypes implement three distributed strategies for the same problem.

---

## Project Overview

Given a set $S$ of $N$ points in $E^d$, a point $p$ **dominates** a point $q$ if $p$ is greater than or equal to $q$ in every coordinate, with strict inequality in at least one. A point is a **maximum** of $S$ if no other point in $S$ dominates it. The maxima problem asks for all such points.

This project investigates:

- Dominance relations between points
- Divide-and-conquer computation of maxima (`MAXIMA2`)
- The recursive `FILTER` merge routine and its 2D sweep base case (`FILTER2`)
- A brute-force $O(N^2 d)$ baseline for correctness and performance comparison
- Empirical runtime behavior across dimension and input size
- Validation against hand-constructed test datasets and a randomized differential test
- Distributed implementations using MPI (ring, fan-in/fan-out, marry-go-round)

---

## Repository Structure

```text
non-dominated-points/
│
├── src/
│   ├── main.cpp               # Benchmark driver: MAXIMA2 suite
│   ├── main_bruteforce.cpp    # Benchmark driver: brute-force suite
│   ├── benchmark_runner.cpp   # Shared timing loop and suite runner
│   ├── benchmark.cpp          # CSV row writer (one row per configuration)
│   ├── dataset.cpp            # Dataset loader shared by drivers and tools
│   ├── dominance.cpp          # Dominance relation
│   ├── point_set.cpp          # Point set validation (dimension consistency)
│   ├── sorting.cpp            # Coordinate-based sorting helpers
│   ├── maxima2.cpp            # MAXIMA2 divide-and-conquer algorithm (in place)
│   ├── brute_force_maxima.cpp # Brute-force O(N^2) maxima
│   ├── filter.cpp             # FILTER / FILTER2 (in place)
│   ├── partition.cpp          # In-place equipartition of V for FILTER
│   ├── split_u.cpp            # In-place threshold split of U for FILTER
│   ├── threshold.cpp          # Threshold extraction from V1
│   └── print.cpp              # Console output helper
│
├── include/                   # Corresponding headers
│
├── mpi_src/
│   ├── mpi_ring_method.cpp            # Ring / round-robin implementation
│   ├── mpi_fan_in_out.cpp             # Fan-out / reduction-tree implementation
│   ├── mpi_marry_go_round_method.cpp  # Marry-go-round implementation
│   └── mpi_items.hpp                  # Flat-buffer conversions shared by the MPI programs
│
├── DataSets/                  # Generated benchmark inputs, grouped by dimension (5D-10D)
│   └── create_testcases.py    # Generator for the benchmark datasets
│
├── validation/
│   ├── validator.cpp          # Loads hand-crafted test datasets
│   ├── validation_main.cpp    # Runs MAXIMA2 against expected output (16 test cases)
│   ├── differential_test.cpp  # Randomized MAXIMA2 vs brute-force comparison
│   ├── dominance_test.cpp     # Dominance and weak dominance on their own
│   ├── edge_case_test.cpp     # Empty, single-point, duplicate and 1D inputs
│   ├── partition_filter_test.cpp # PartitionV / SplitU / FILTER contracts
│   ├── dataset_test.cpp       # Loader refusal paths
│   ├── benchmark_csv_test.cpp # CSV upsert and missing-dataset handling
│   ├── antichain_test.cpp     # The worst-case generator is an antichain
│   └── test_datasets/         # Test1.txt .. Test16.txt, MPI_Test.txt
│
├── plots/
│   ├── plot_results.py        # Sequential runtime plots from benchmark CSVs
│   ├── plot_mpi_results.py    # Speedup / scaling plots from the MPI CSV
│   ├── plot_analysis.py       # Growth, advantage, parallel and memory analyses
│   ├── plot_mpi_timing.py     # Phase breakdown of the marry-go-round merge
│   ├── plot_worst_case.py     # Antichain (every point maximal) analysis
│   ├── benchmark_data.py      # Shared paths/loading for the plot scripts
│   ├── data/                  # Measurement inputs for the analysis plots
│   └── figures/               # Output plots
│
├── results/                   # Benchmark CSVs (generated, not tracked)
├── tools/large_benchmark.cpp  # Million-point and peak-memory measurements
├── tools/worst_case_benchmark.cpp  # Antichain timings (every point maximal)
├── scripts/mpi_marry_go.sh    # SLURM job script for the marry-go-round benchmark
├── scripts/mpi_timing_sweep.sh   # Local phase-breakdown sweep for the merge
├── scripts/worst_case_sweep.sh   # Antichain sweep behind the worst-case figure
├── review_report.pdf          # Benchmark notes and discussion (source: .tex)
├── Thesis_Report.pdf
├── CMakeLists.txt
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

A point that is dominated by any other point can never be a maximum and is discarded. Exact duplicates do not dominate each other, so a point that occurs twice contributes two maxima.

Points are stored as `Item` (a coordinate vector), and a `PointSet` carries its dimension together with the points. Algorithms that index coordinates accept a `PointSet` and validate it, so a dimension that does not match the data is rejected with `std::invalid_argument` instead of reading out of bounds.

### 2. Random Point Generation

[`DataSets/create_testcases.py`](DataSets/create_testcases.py) writes the benchmark inputs: `N` points with `D` coordinates, each drawn uniformly from `[0, 1]` and rounded to six decimals for storage. Every dimension directory contains `Test1` … `Test7` with 50, 100, 500, 1 000, 10 000, 100 000 and 1 000 000 points respectively.

### 3. Brute-Force Maxima

`BruteForceMaxima(pointSet)` checks every point against every other point directly ($O(N^2 d)$) and keeps the ones that are never dominated. It is the reference implementation, the performance baseline, and the fallback `MAXIMA2` uses when a recursive split cannot separate the points (e.g. all points tie on the split coordinate).

### 4. MAXIMA2 — Divide-and-Conquer Maxima

`MAXIMA2(pointSet)` sorts the points by their last coordinate itself, so callers do not have to know about that requirement. It then:

1. Splits at the median, adjusted so that tied values on the split coordinate all land on the same side (preserving correctness). If the split cannot separate the set at all, it falls back to `BruteForceMaxima`.
2. Recursively computes the maxima of each half, giving `M1` (lower half) and `M2` (upper half).
3. **Merges**: every point of `M2` survives automatically, since it was sorted to dominate on the split coordinate. A point of `M1` survives only if no point of `M2` dominates it on the remaining `d-1` coordinates — this check is delegated to `FILTER`.

Both steps work in place on disjoint ranges of one contiguous buffer. Each recursive call partitions its range and compacts its result to the front, so no level copies its input; `MAXIMA2` allocates exactly one working copy of the point set and returns the maxima already compacted.

### 5. FILTER — Recursive Merge Step

`FILTER(U, V, d)` computes, for two point sets `U` and `V`, the subset of `U` not dominated by any point of `V`. It recurses by equipartitioning `V` and splitting `U` at the corresponding threshold, recursing three ways (`U2` vs `V2`, `U1` vs `V1`, and `U1` vs `V2` with one fewer dimension). The third sub-problem is applied to the survivors of the second, which is the same intersection without materialising two intermediate sets. It bottoms out at a linear-time 2D sweep (`FILTER2`) once only two coordinates remain relevant, plus a few cheap base cases (`U` empty, `V` empty, `|V| = 1`, `d = 1`).

All of this happens in place: `U` and `V` are disjoint ranges of one buffer, survivors are compacted to the front of `U`, and `V` is only ever reordered, never discarded, because the merge still needs all of `M2`.

Because the coordinate that `FILTER` has dropped from the comparison already dominates strictly, `FILTER` uses **weak** dominance on the coordinates it still compares: a point that merely ties is removed. Using strict dominance there would keep points that are dominated on the dropped coordinate.

### 6. Validation

Fifteen tests run under CTest. Each has a time bound, so a deadlock shows up as one failed test in seconds instead of a run that has to be interrupted.

**Algorithms.** `validator` loads 16 hand-built datasets from `validation/test_datasets/` (input points, dimension and expected maxima) and compares `MAXIMA2`'s output with the expected set, independent of ordering. `differential_test` compares `MAXIMA2` against `BruteForceMaxima` over 13 104 generated cases (dimensions 2–8, point counts 2–40, integer and fractional coordinate ranges chosen to produce ties and duplicates) and checks that inconsistent point sets are rejected. Both use a fixed seed, so a failure is reproducible.

**Primitives and edge cases.** `dominance_relation` pins down `Dominates` and `WeaklyDominates` on their own, including the rule that a tie is not an improvement, antisymmetry and transitivity — a difference between the two algorithms cannot expose a mistake they share. `edge_cases` covers what the random sweep never generates: the empty set, a single point, exact duplicates, every point in one tie group, one dimension and twelve dimensions, each against a hand-written answer rather than only against the baseline. `partition_filter` states the contracts of `PartitionV`, `SplitU` and `FILTER` directly, including that `FILTER` removes points that merely tie on the compared coordinates and never drops a point of `V`.

**Pipeline.** `dataset_loader` covers the loader's refusal paths; `benchmark_pipeline` covers the CSV upsert (one row per configuration, header written once) and the suite's handling of a missing dataset, which must be skipped and reported rather than recorded as a zero timing; `antichain_generator` checks that the worst-case input really is an antichain and that both algorithms return every point.

**Distributed.** The three MPI programs are run on a fixture with a known answer (4 maxima) at the process counts where the token passing wraps around: `MPI_Marrygomethod` at 1, 2 and 3 ranks, `MPI_Ringmethod` and `MPI_FanInOut` at 2 and 3. Their results are checked by regular expression against the programs' own output, so a program that computes the wrong set fails even though it exits successfully.

```bash
cd build && ctest --output-on-failure
```

### 7. Benchmarking

`Maxima_method` runs `MAXIMA2` and `brute_force` runs `BruteForceMaxima` over the datasets in `DataSets/`, taking the median of ten repetitions per configuration and writing to `results/maxima_benchmark.csv` and `results/brute_force_benchmark.csv` respectively. Both drivers share the same loader and timing code.

Both drivers cover `Test1` through `Test6` (50 to 100 000 points); `Test7` holds 1 000 000 points and is reserved for the parallel experiments, since the quadratic baseline cannot process it. Each configuration occupies exactly one row, so re-running a dimension replaces that dimension's measurements instead of appending duplicates, while other dimensions are preserved. Every row carries the median of the repetitions together with the fastest and slowest one, so plots can show how much a single measurement varies. A dataset that cannot be read is reported and skipped rather than recorded as a zero timing, and the driver then exits with a non-zero status.

`large_benchmark` measures the configurations the drivers cannot cover: the million-point `Test7` datasets and the peak memory needed on top of the input. It handles one dataset per process, so the resident-memory high-water mark belongs to that run alone, and appends a row to the CSV it is given.

`worst_case_benchmark` measures the opposite endpoint to the random datasets: an antichain, where no point dominates another and every point is maximal. It builds the input in memory from integer cuts that all sum to the same constant, so domination is impossible by construction, and it refuses to write a row unless the output really has `N` points. One configuration and one algorithm per process.

The marry-go-round program writes a second CSV next to its summary file: `results/mpi_marry_go_round_timing.csv` splits the run into local brute force, cross-filter, communication and barrier wait, with scatter, gather, the rank-0 read time, bytes exchanged and a load-imbalance ratio. The breakdown is taken from the rank that determined the wall time, so its parts add up to that total; the busiest rank's communication is stored separately.

The plot scripts read the CSVs and never time anything themselves:

| Script | Figures |
|--------|---------|
| `plots/plot_results.py` | Runtime comparison per dimension, with min/max error bars |
| `plots/plot_analysis.py` | Algorithmic advantage, cost per point, normalised runtime, fitted growth exponent, maxima fraction, sequential-versus-parallel, peak memory |
| `plots/plot_mpi_results.py` | Cluster speed-up and scaling from the MPI CSV |
| `plots/plot_mpi_timing.py` | Phase breakdown of the merge, absolute and as a share of wall time |
| `plots/plot_worst_case.py` | Antichain against uniform data: runtime, fitted exponent, penalty |

`plots/plot_analysis.py` and the two newer scripts also read the small measurement files in `plots/data/` (local MPI against sequential `MAXIMA2`, the million-point/memory run, the MPI phase breakdown, and the antichain sweep); those are committed so the figures can be reproduced without re-running the benchmarks. The measurements and the discussion around them — including what is still missing — are in [review_report.pdf](review_report.pdf) (source: [review_report.tex](review_report.tex)).

---

## Build Instructions

### Prerequisites

- C++17-capable compiler (GCC / Clang / MSVC)
- CMake ≥ 3.15
- An MPI implementation (optional; the MPI programs are skipped when it is absent)
- Python 3 with pandas and matplotlib (for plotting benchmark results)

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Relevant CMake options: `NDP_ENABLE_MPI` (default `ON`), `NDP_BUILD_TESTS` (default `ON`), `NDP_SANITIZE` (default `OFF`) and `NDP_WARNINGS_AS_ERRORS` (default `OFF`, enabled in CI).

### Executables

| Executable            | Purpose                                                        |
|------------------------|----------------------------------------------------------------|
| `Maxima_method`        | Benchmark suite for `MAXIMA2` over one dimension folder        |
| `brute_force`          | Benchmark suite for the brute-force baseline                   |
| `large_benchmark`      | Million-point and peak-memory measurement (one dataset/process) |
| `worst_case_benchmark` | Antichain timing for one algorithm and configuration            |
| `validator`            | 16 hand-built correctness cases                                |
| `differential_test`    | Randomized comparison against the brute-force reference        |
| `MPI_Ringmethod`       | Ring / round-robin distributed maxima                          |
| `MPI_FanInOut`         | Fan-out / reduction-tree distributed maxima                    |
| `MPI_Marrygomethod`    | Marry-go-round distributed maxima                              |

### Run

All benchmark paths are relative to the build directory.

```bash
./Maxima_method 5D_Data              # MAXIMA2 benchmark for the 5D datasets
./brute_force ../DataSets/5D_Data    # brute-force benchmark for the same data
cd .. && ctest --test-dir build --output-on-failure   # the whole suite

# million-point run and peak memory (one dataset per process)
cd build && ./large_benchmark ../plots/data/memory_and_large_n.csv \
                               ../DataSets/10D_Data/Test7.txt

# worst case: an antichain, where every point is maximal
./worst_case_benchmark ../plots/data/worst_case_all_maxima.csv 10 10000 maxima2
cd .. && scripts/worst_case_sweep.sh build/worst_case_benchmark \
                                    plots/data/worst_case_all_maxima.csv

python3 plots/plot_results.py && python3 plots/plot_analysis.py \
      && python3 plots/plot_worst_case.py
```

The MPI programs take a dataset path:

```bash
mpiexec -n 4 ./MPI_FanInOut ../DataSets/10D_Data/Test6.txt
```

`scripts/mpi_marry_go.sh` drives the marry-go-round benchmark across process counts and repetitions on a SLURM cluster; `scripts/mpi_timing_sweep.sh` is its local counterpart and produces the phase breakdown (run it from a build directory, since the program writes to `../results/`). `python3 plots/plot_mpi_timing.py` turns that file into a figure.

---

## Experimental Results

Benchmark results are written to `results/maxima_benchmark.csv` and `results/brute_force_benchmark.csv` and plotted by `plots/plot_results.py` into `plots/figures/runtime_comparison_<D>D.png`, showing execution time as a function of point count for each tested dimension. The MPI measurements are plotted by `plots/plot_mpi_results.py` into `plots/figures/speedup_all_dimensions.png`, `plots/figures/problem_size_scaling_all_processors.png` and `plots/figures/execution_time_vs_processors_all_dimensions.png`.

### In-place implementation

Measured on one machine (Apple silicon, AppleClang, `-O3 -DNDEBUG`, median of 3–10 repetitions), `MAXIMA2` over the in-place implementation versus the previous version that copied subsets at every recursion level:

| Dataset | Points | Dimension | Before | After | Speed-up |
|---------|-------:|----------:|-------:|------:|---------:|
| `5D_Data/Test4` | 1 000 | 5 | 5.34 ms | 0.45 ms | 12× |
| `5D_Data/Test5` | 10 000 | 5 | 48.0 ms | 4.55 ms | 11× |
| `5D_Data/Test6` | 100 000 | 5 | 495 ms | 35.5 ms | 14× |
| `7D_Data/Test5` | 10 000 | 7 | 176 ms | 9.13 ms | 19× |
| `10D_Data/Test4` | 1 000 | 10 | 17.9 ms | 0.91 ms | 20× |
| `10D_Data/Test5` | 10 000 | 10 | 484 ms | 26.2 ms | 18× |

Peak resident memory for the 10 000-point, 10-dimensional case fell from 11.4 MB to 3.7 MB. The maxima are identical in every case; the improvement comes from removing the per-level copies and the materialised intersections, not from a change in the algorithm.

The *Before* column was measured from the pre-rewrite build; the *After* column is the current suite's median of ten repetitions.

The CSVs are generated rather than committed, because they depend on the machine and compiler used.

### Scaling, memory and the sequential/parallel comparison

In 5D–10D the measured runtime grows like $N^{0.95}$ to $N^{1.36}$ between 1 000 and 100 000 points, well below the $N^{1.33}$–$N^{1.89}$ implied by the worst-case bound; the share of the input that survives (0.8 % in 5D, 25.9 % in 10D at 100 000 points) explains why the quadratic baseline degrades faster in higher dimensions. A million points take 0.43 s in 5D and 7.0 s in 10D with a peak of 139–201 MB, and on one machine eight processes of the MPI brute force are still slower than one core of `MAXIMA2` at 10D.

The merge itself is timed by phase: on 10D/100 000 the cross-filter stays flat at 359–413 ms from two to eight processes while the local brute force falls 22.7×, so the decomposition is bounded by a phase that does not scale — roughly a 33× ceiling at any process count. Small configurations are limited by synchronisation instead: at 5D/10 000 and eight processes, exchange and barrier are 69 % of the wall time.

On an antichain, where every point is maximal, `MAXIMA2` is 2.6–9.7× slower than on uniform data, its growth exponent moves onto the bound (1.31–1.55 against 1.33–1.89), and the baseline becomes exactly quadratic (exponent 1.99–2.07).

The figures for these statements are in [`plots/figures/`](plots/figures) (`algorithmic_advantage.png`, `scaling_exponent.png`, `maxima_fraction.png`, `sequential_vs_parallel.png`, `memory_and_large_n.png`, `mpi_time_breakdown.png`, `worst_case_all_maxima.png`), and the full discussion — including the measurement caveats and what is still missing — is in [review_report.pdf](review_report.pdf).

---

## Roadmap

The sequential maxima implementation and three MPI prototypes are in place. Open work:

- **Parallel MAXIMA2 / FILTER implementation** — parallelize the divide-and-conquer recursion itself: the two recursive `MAXIMA2` calls on `S1`/`S2` are independent and can run concurrently, as can `FILTER`'s three-way recursive split (`U2` vs `V2`, `U1` vs `V1`, `U1` vs `V2`), with attention to load balancing since the three branches aren't generally the same size.
  - **CUDA** — parallelize the dominance checks and the sweep/merge steps on the GPU for large point sets.
  - **Multithreading (e.g. `std::thread` / OpenMP)** as a lighter-weight intermediate step before CUDA, if useful for shared-memory experiments.
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

See [LICENSE](LICENSE). The project is intended for academic and research purposes.
