#ifndef KNAPSACKLIB
#define KNAPSACKLIB
#include <array>
#include <string>

#define LINE_DIMENSION 50

struct KnapsackObject {
    double profit;
    double weight;
};

struct Problem {
    KnapsackObject* problem;
    int nElements;
    int knapsackWeigth;
};

bool compareElements(const KnapsackObject e1, const KnapsackObject e2);

struct KnapsackElementSolution {
    int id;
    bool in_knapsack;
};

bool compareElementSolution(const KnapsackElementSolution s1, const KnapsackElementSolution s2);

typedef struct elemSolution* sol;

struct solutionList {
    sol Head;
    sol Tail;
};

Problem* readFile(std::string fileName);

void sortKnapsack(Problem* problem);


#endif