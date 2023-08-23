CC=mpic++

CFLAGS=-c -Wall

MAKE=make

BRANCHBOUNDIR=branchbound
KNAPSACKDIR=knapsack

branch_to_build = $(wildcard branchbound/*.o)
knap_to_build = $(wildcard knapsack/*.o)

.PHONY: all

all: prog clean

prog: branchbound.o knapsack.o main.o
	$(CC) $(branch_to_build) $(knap_to_build) main.o -o branchbound.out

branchbound.o:
	$(MAKE) -C $(BRANCHBOUNDIR)

knapsack.o:
	$(MAKE) -C $(KNAPSACKDIR)

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean: cleanbranchbound cleaknapsack cleanmain

cleanmain:
	rm -rf *.o

cleanbranchbound:
	 $(MAKE) -C $(BRANCHBOUNDIR) clean

cleaknapsack:
	$(MAKE) -C $(KNAPSACKDIR) clean