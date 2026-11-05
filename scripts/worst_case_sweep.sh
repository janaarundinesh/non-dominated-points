#!/bin/bash
# Measures MAXIMA2 and the quadratic baseline on an antichain, the input on
# which every point is maximal and nothing can be pruned early.
#
# The brute-force baseline is quadratic with no early exit on this input, so it
# is stopped at 10 000 points; the million-point datasets are far out of reach.
# Run from any directory, after building worst_case_benchmark:
#
#   scripts/worst_case_sweep.sh build/worst_case_benchmark plots/data/worst_case_all_maxima.csv

set -euo pipefail

BINARY=${1:?usage: worst_case_sweep.sh <path/to/worst_case_benchmark> <output.csv>}
OUTPUT=${2:?usage: worst_case_sweep.sh <path/to/worst_case_benchmark> <output.csv>}

DIMS=${DIMS:-"5 6 7 8 9 10"}
SIZES=${SIZES:-"50 100 500 1000 10000"}
LARGE_SIZES=${LARGE_SIZES:-"100000"}
MAX_BRUTE_FORCE=${MAX_BRUTE_FORCE:-10000}

mkdir -p "$(dirname "$OUTPUT")"

for d in $DIMS; do
    for n in $SIZES; do
        "$BINARY" "$OUTPUT" "$d" "$n" maxima2

        if [ "$n" -le "$MAX_BRUTE_FORCE" ]; then
            "$BINARY" "$OUTPUT" "$d" "$n" brute_force
        fi
    done

    for n in $LARGE_SIZES; do
        "$BINARY" "$OUTPUT" "$d" "$n" maxima2
    done
done
