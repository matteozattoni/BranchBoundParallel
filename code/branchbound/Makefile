# Branch and Bound Makefile
CC=mpic++

CFLAGS= -fopenmp -c -Wall

.PHONY: clean

BUILDDIR = build

VPATH = mpi/

all: mpi.o openmp.o seq_branchbound.o branchbound.o 

branchbound.o: branchbound.cpp
	$(CC) $(CFLAGS) branchbound.cpp


mpi.o: mpi/mpimessage.cpp mpi/masterpoolmanager.cpp mpi/workpoolmanager.cpp mpi/commworldmanager.cpp  mpi/mpibranchboundmanager.cpp
	$(CC) $(CFLAGS) mpi/mpimessage.cpp mpi/masterpoolmanager.cpp mpi/workpoolmanager.cpp mpi/commworldmanager.cpp mpi/mpibranchboundmanager.cpp

openmp.o: openmp/openmpmanager.cpp
	$(CC) $(CFLAGS) openmp/openmpmanager.cpp


seq_branchbound.o: sequential/seq_branchbound.cpp
	$(CC) $(CFLAGS) sequential/seq_branchbound.cpp

clean:
	rm -rf *.o
