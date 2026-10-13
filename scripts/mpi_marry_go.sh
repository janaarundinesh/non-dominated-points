#!/bin/bash
#SBATCH --nodes=1
#SBATCH --partition=compute2011
#SBATCH --exclusive

module load mpi/openmpi/4.1.0

mpic++ -std=c++17 -O2 ../mpi_src/mpi_marry_go_round_method.cpp -o MPI_Marrygomethod

# Must be submitted from build/ (SLURM starts the job in the submit directory)
if [ ! -x ./MPI_Marrygomethod ]; then
    echo "MPI_Marrygomethod not found in $(pwd). Build it with CMake and submit from build/."
    exit 1
fi

mkdir -p ../results

NP=${NP:-"1 2 3 4 5 6 7 8 9 10 15 20"}
REPS=${REPS:-3}
DIMS=${DIMS:-"5 6 7 8 9 10"}
TESTS=${TESTS:-"1 2 3 4 5 6"}

for d in $DIMS; do
    for t in $TESTS; do
        file="../DataSets/${d}D_Data/Test${t}.txt"
        for np in $NP; do
            for rep in $(seq 1 "$REPS"); do
                mpirun -np "$np" ./MPI_Marrygomethod "$file"
            done
        done
    done
done