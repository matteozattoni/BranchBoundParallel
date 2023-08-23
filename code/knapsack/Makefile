# Branch and Bound Makefile

CC=mpic++

CFLAGS=-c -Wall

.PHONY: clean

all: knapsack.o

knapsack.o: knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsacksolution.cpp knapsackmemorymanager.cpp knapsack.cpp
	$(CC) $(CFLAGS) knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsacksolution.cpp knapsackmemorymanager.cpp knapsack.cpp

clean:
	rm -rf *.o
