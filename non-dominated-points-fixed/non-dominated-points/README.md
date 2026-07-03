# Non-Dominated-Points Thesis Project

A C++ implementation and experimental study of Pareto-based filtering techniques for the **0/1 Knapsack Problem**.

This project explores how dominance relations and Pareto-optimal filtering can be used to reduce the search space of candidate solutions. The implementation includes both a conventional dominance-checking approach and an optimized **Sweep-and-Filter** method based on Pareto efficiency.

---

## Project Overview

The 0/1 Knapsack Problem is a classical combinatorial optimization problem where a subset of items must be selected to maximize profit while respecting a weight constraint.

This thesis investigates:

- Pareto dominance concepts
- Non-dominated item filtering
- Sweep-and-Filter techniques
- Performance improvements over brute-force dominance checking
- Practical implementation in modern C++

---

## Repository Structure

```text
pareto-knapsack-thesis/
│
├── src/
│   ├── main.cpp
│   ├── generate_data.cpp
│   ├── dominance.cpp
│   ├── sorting.cpp
│   ├── maxprofitseen.cpp
│   └── knapsack.cpp
│
├── include/
│   ├── Item.hpp
│   ├── generate_data.hpp
│   ├── dominance.hpp
│   ├── sorting.hpp
│   ├── maxprofitseen.hpp
│   └── knapsack.hpp
│
├── build/
│
├── docs/
│   └── thesis_documentation
│
└── README.md
```

---

## Implemented Concepts

### 1. Random Item Generation

Generates a collection of items with:

- Weight
- Profit

Example:

| Weight   | Profit   |
|----------|----------|
| 5        | 12       |
| 7        | 20       |
| 3        | 9        |

---

### 2. Dominance Relation

An item **A** dominates item **B** if:

```text
A.weight ≤ B.weight
A.profit ≥ B.profit
```

with at least one strict inequality.

Dominated items can be removed without affecting the optimal solution space.

---

### 3. Conventional Dominance Filtering

A brute-force method that compares every item against all others.

Complexity:O(n²)

```

This approach serves as a correctness baseline.

---

### 4. Sweep-and-Filter Method

The optimized filtering strategy:

1. Sort items by weight.
2. Traverse from smallest to largest weight.
3. Maintain the maximum profit encountered.
4. Discard items that are dominated by previously seen items.

Complexity:

```
Sorting : O(n log n)
Sweep   : O(n)

Total   : O(n log n)
```

---

## Example Workflow

### Generated Items

```
Weight : 5, Profit : 9
Weight : 2, Profit : 10
Weight : 6, Profit : 15
Weight : 3, Profit : 8
Weight : 4, Profit : 12
```

### After Sorting

```
Weight : 2, Profit : 10
Weight : 3, Profit : 8
Weight : 4, Profit : 12
Weight : 5, Profit : 9
Weight : 6, Profit : 15
```

### After Sweep-and-Filter

```
Weight : 2, Profit : 10
Weight : 4, Profit : 12
Weight : 6, Profit : 15
```

---

## Build Instructions

### Prerequisites

- C++17 or later
- CMake (recommended)
- GCC / Clang / MSVC

### Clone Repository

```bash
git clone https://github.com/janaarundinesh/non-dominated-points.git
cd non-dominated-points
```

### Build

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

### Run

```bash
./non-dominated-points
```

---

## Experimental Results

### Comparison of Filtering Approaches

| Method | Complexity |
|----------|----------|
| Conventional Dominance Check | O(n²) |
| Sweep-and-Filter | O(n log n) |

<!-- TODO: Insert performance comparison table -->

<!-- TODO: Insert benchmark screenshots -->

<!-- TODO: Insert execution time graphs -->

<!-- TODO: Insert memory consumption analysis -->

---

## Sample Output

<!-- TODO: Insert terminal output screenshot -->

<!-- TODO: Insert example generated dataset -->

<!-- TODO: Insert filtered Pareto frontier output -->

---

## Algorithm Visualization

### Dominance Filtering

<!-- TODO: Insert dominance relation diagram -->

### Sweep-and-Filter Process

<!-- TODO: Insert sweep-and-filter workflow diagram -->

### Pareto Frontier

<!-- TODO: Insert Pareto frontier graph -->

---

## Thesis Objectives

- Understand Pareto dominance in optimization.
- Implement efficient filtering strategies.
- Compare brute-force and optimized approaches.
- Evaluate computational complexity.
- Analyze scalability for larger datasets.

---

## Future Work

Potential extensions include:

- Complete dynamic programming implementation for 0/1 Knapsack.
- Pareto-optimal state-space reduction.
- Multi-objective knapsack variants.
- Parallelized filtering algorithms.
- Large-scale benchmarking.

---

## Documentation

<!-- TODO: Add link to thesis PDF -->

<!-- TODO: Add link to presentation slides -->

<!-- TODO: Add link to final report -->

---

## Author

**Arun Dinesh Jana**

Master's Thesis Project

<!-- TODO: Add university name -->

<!-- TODO: Add department/program -->

<!-- TODO: Add supervisor information -->

---

## License

This project is intended for academic and research purposes.

<!-- TODO: Add chosen license (MIT, GPL, Apache 2.0, etc.) -->

---

## Acknowledgements

Special thanks to:

<!-- TODO: Add supervisor acknowledgement -->

<!-- TODO: Add university acknowledgement -->

<!-- TODO: Add research group acknowledgement -->

---
