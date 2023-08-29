#! /bin/sh

# Job name:
#SBATCH --job-name=br_bound
#
# Partition:
#SBATCH --partition=broadwell
#
# Number of tasks needed for use case:
#SBATCH -n 360
#
# Number of nodes and how many task per each node
## SBATCH -N4 --ntasks-per-node=32
#
# Wall clock limit hours:minutes:seconds
## SBATCH --time=00:30:00
#
# Log
#SBATCH -o %j.log
#
# Error
#SBATCH -e %j.err

## Command(s) to run (example):
#eval spack load --sh openmpi@4.1.3%gcc@9.4.0
## srun /bin/hostname ## print the hostname
srun --mpi=pmix ./branchbound.out ##pmix is the slurm launcher (like mpirun) 