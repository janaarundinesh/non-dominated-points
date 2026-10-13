#!/bin/bash
#SBATCH --nodes=1
#SBATCH --partition=compute2011
#SBATCH --exclusive
#SBATCH --job-name=marrygo_maxima
#SBATCH --output=marrygo_bench_%j.log

module load mpi/openmpi/4.1.0

# Must be submitted from build/ (SLURM starts the job in the submit directory)
if [ ! -x ./MPI_Marrygomethod ]; then
    echo "MPI_Marrygomethod not found in $(pwd). Build it with CMake and submit from build/."
    exit 1
fi

mkdir -p ../results

MAX_NP=${NP:-"1 2 3 4 5 6 7 8 9 10 15 20"}
REPS=${REPS:-3}
DIMS=${DIMS:-"5 6 7 8 9 10"}
TESTS=${TESTS:-"1 2 3 4 5 6 7"}

for d in $DIMS; do
    for t in $TESTS; do
        file="../DataSets/${d}D_Data/Test${t}.txt"

        for np in $NP; do
            for rep in $(seq 1 "$REPS"); do
                start_time=$(date +%s%N)

                mpirun -np "$np" ./MPI_Marrygomethod "$file"

                end_time=$(date +%s%N)
                elapsed_time=$(( (end_time - start_time) / 1000000 ))
                echo "${d}D Test${t} | np=${np} rep=${rep} | wall time incl. startup: ${elapsed_time} ms"
            done
        done
    done
done