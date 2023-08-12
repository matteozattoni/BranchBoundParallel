CC=mpic++

CFLAGS=-c -Wall

MAKE=make

BRANCHBOUNDIR=./branchbound
KNAPSACKDIR=./knapsack

all: prog

prog: branchbound.o knapsack.o main.o
	$(CC) main.o -o prog

branchbound.o:
	$(MAKE) -C $(BRANCHBOUNDIR) compile

knapsack.o:
	$(MAKE) -C $(KNAPSACKDIR) compile

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean: cleanbranchbound cleaknapsack cleanmain

cleanmain:
	rm -rf *.o

cleanbranchbound:
	 $(MAKE) -C $(BRANCHBOUNDIR) clean

cleaknapsack:
	$(MAKE) -C $(KNAPSACKDIR) clean