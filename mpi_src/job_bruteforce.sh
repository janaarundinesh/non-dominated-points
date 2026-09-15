#!/bin/bash

mpic++ -std=c++17 \
    mpi_ring_method.cpp \
    ../src/dominance.cpp \
    ../src/brute_force_maxima.cpp \
    -I../include \
    -o MPI_RingMethod

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo "Compilation successful."
for p in 1 2 4 5 8 10; do
    time mpirun -np $p ./MPI_RingMethod
done