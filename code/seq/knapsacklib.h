#ifndef KNAPSACKLIB
#define KNAPSACKLIB
#include <array>

struct knapsackelem {
    double profit;
    double weight;
};

typedef struct knapsackelem* elem;

bool compareElements(const knapsackelem e1, const knapsackelem e2);

struct knapsacklist {
    elem Head;
    elem Tail;
};

struct knapsackproblem {
    knapsackelem* elements;
};


struct elemSoloution {
    int id;
    bool in_knapsack;
};

typedef struct elemSolution* sol;

struct solutionList {
    sol Head;
    sol Tail;
};

#endif