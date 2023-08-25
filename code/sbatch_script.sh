#! /bin/sh

# Job name:
#SBATCH --job-name=bra_bound
#
# Partition:
#SBATCH --partition=broadwell
#
# Number of nodes needed for use case:
#SBATCH --nodes=4
#
# Wall clock limit 4 hours:
#SBATCH --time=04:00:00
#
# Log
#SBATCH -o %j.log
#
# Error
#SBATCH -e %j.err

## Command(s) to run (example):
#eval spack load --sh openmpi@4.1.3%gcc@9.4.0
srun /bin/hostname ## print the hostname
srun --mpi=pmix ./branchbound.out ##pmix is the slurm launcher (like mpirun) 