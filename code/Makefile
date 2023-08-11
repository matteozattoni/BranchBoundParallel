CC=mpic++

CFLAGS=-c -Wall

all: prog

prog: branchbound.o knapsack.o main.o
	$(CC) branchbound.o knapsack.o main.o -o prog

branchbound.o: branchbound.cpp
	$(CC) $(CFLAGS) branchbound.cpp

knapsack.o: knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsackmemory.cpp knapsackmemorymanager.cpp knapsacksolution.cpp knapsack.cpp
	$(CC) $(CFLAGS) knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsackmemory.cpp knapsackmemorymanager.cpp knapsacksolution.cpp knapsack.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm -rf *.o