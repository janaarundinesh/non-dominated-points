#!/bin/bash
# Produces results/mpi_marry_go_round_timing.csv: one row per
# (dimension, points, processes) with the wall time split into local work,
# cross-filter work, communication, barriers, scatter and gather.
#
# The binary writes relative to its working directory ("../results"), so run it
# from a build directory whose parent contains results/. This is the local
# counterpart of mpi_marry_go.sh, which does the same on the cluster.
#
#   cd build && ../scripts/mpi_timing_sweep.sh

set -euo pipefail

NP=${NP:-"1 2 4 8"}
REPS=${REPS:-3}
DATASETS=${DATASETS:-"5D_Data/Test5.txt 10D_Data/Test5.txt 10D_Data/Test6.txt"}

if [ ! -x ./MPI_Marrygomethod ]; then
    echo "MPI_Marrygomethod not found in $(pwd). Build it with CMake first."
    exit 1
fi

mkdir -p ../results

for dataset in $DATASETS; do
    for np in $NP; do
        for rep in $(seq 1 "$REPS"); do
            mpirun -np "$np" ./MPI_Marrygomethod "../../DataSets/$dataset"
        done
    done
done
