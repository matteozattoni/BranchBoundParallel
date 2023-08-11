# Branch and Bound Makefile
CC=mpic++

CFLAGS=-c -Wall

branchboundprog: branchbound.o 

branchbound.o: branchbound.cpp
	$(CC) $(CFLAGS) branchbound.cpp -o branchbound.o

cleanbranchbound:
	rm -rf *.o
