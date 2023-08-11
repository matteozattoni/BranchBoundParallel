# Branch and Bound Makefile

include ../branchbound/Makefile

CC=mpic++

CFLAGS=-c -Wall


knapsackprog: knapsackprog.o branchbound.o

knapsackprog.o: knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsackmemory.cpp knapsackmemorymanager.cpp knapsacksolution.cpp knapsack.cpp
	$(CC) $(CFLAGS) knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsackmemory.cpp knapsackmemorymanager.cpp knapsacksolution.cpp knapsack.cpp

cleanknapsack:
	rm -rf *.o
