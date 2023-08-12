CC=mpic++

CFLAGS=-c -Wall

MAKE=make

BRANCHBOUNDIR="./branchbound"
KNAPSACKDIR="./knapsack"

all: $(BRANCHBOUNDIR) $(KNAPSACKDIR) prog

prog: main.o
	$(CC) main.o -o prog

$(BRANCHBOUNDIR):
	$(MAKE) -C $@

$(KNAPSACKDIR):
	$(MAKE) -C $@

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean: $(BRANCHBOUNDIR) $(KNAPSACKDIR) cleanmain
	rm -rf *.o

cleanmain:
	rm -rf *.o

$(BRANCHBOUNDIR):
	$(MAKE) -C $@ clean

$(KNAPSACKDIR):
	$(MAKE) -C $@ clean