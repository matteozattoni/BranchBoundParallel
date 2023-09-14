# BranchBoundParallel
 Branch and Bound implementation using MPI (v. 4.1.5)

### Build image and launch container
1. wget https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.5.tar.gz
2. docker-compose up

#### container info
container name: debian_openmpi

container username: mpi

container psw: mpi


### Compile whole project
cd /code

make

#### Clean output file
cd /code

make clean

#### Change workpool size
file: /code/branchbound/mpi/mpiexceptions.h

constant name: WORKPOOL_WORKER

#### Change dataset file path
file: /code/knapsack/knapsackmemorymanager.h

constant name: FILEPATH
