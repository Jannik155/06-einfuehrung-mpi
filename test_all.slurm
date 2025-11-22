#!/bin/bash
#SBATCH --job-name=test-all-mpi
#SBATCH --output=test-all-mpi-%j.out
#SBATCH --ntasks=8
#SBATCH --time=00:05:00
#SBATCH --partition=west

# Kompilieren
make

echo "Starte MPI-Tests für verschiedene Prozesszahlen..."
echo ""

for np in 1 2 3 4 8; do
    echo "===== Starte mit np=$np ====="
    mpirun -np $np ./timempi.x
    echo ""
done

echo "Alle Tests fertig."
