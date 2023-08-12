# Branch and Bound Makefile

CC=mpic++

CFLAGS=-c -Wall


knapsackprog: knapsack.o

knapsackprog.o: knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsacksolution.cpp knapsack.cpp
	$(CC) $(CFLAGS) knapsackresult.cpp knapsackproblem.cpp knapsackbranch.cpp knapsacksolution.cpp knapsack.cpp -o knapsack.o

clean:
	rm -rf *.o
