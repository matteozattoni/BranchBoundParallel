# Branch and Bound Makefile

CC=mpic++

CFLAGS=-c -Wall


compile: knapsack.o

knapsackprog.o: knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsacksolution.cpp knapsack.cpp
	$(CC) $(CFLAGS) knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsacksolution.cpp knapsack.cpp

clean:
	rm -rf *.o
