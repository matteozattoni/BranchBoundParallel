
#include "knapsacklib.h"
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>

using namespace std;

Problem* readFile(std::string fileName)
{
    Problem* problem = (Problem*) malloc(sizeof(Problem));
    char line[LINE_DIMENSION];
    const char *delimeter = " ";
    char *val;
    ifstream file(fileName);

    // Get Problem Dimension and Knapsack Total Capacity
    file.getline(line, sizeof(line));
    val = strtok(line, delimeter);
    problem->nElements = atoi(val);
    problem->problem = (KnapsackObject *) calloc(problem->nElements, sizeof(struct KnapsackObject));

    val = strtok(NULL, delimeter);
    problem->knapsackWeigth = atoi(val);

    // get istance
    for (size_t i = 0; i < problem->nElements; i++)
    {
        char line[LINE_DIMENSION];
        char *val;
        int profit, weight;
        if (!file.eof())
        {
            file.getline(line, sizeof(line));
            val = strtok(line, delimeter);
            profit = atoi(val);
            val = strtok(NULL, delimeter);
            weight = atoi(val);
            problem->problem[i].profit = profit;
            problem->problem[i].weight = weight;
        }
    }

    file.close();
    return problem;
}

void sortKnapsack(Problem* problem)
{
    sort(problem->problem, problem->problem + problem->nElements, compareElements);
}

bool compareElements(const KnapsackObject e1, const KnapsackObject e2) {
    double val1 = (double) e1.profit / (double) e1.weight;
    double val2 = (double) e2.profit / (double) e2.weight;
    return val1 > val2; 
}

bool compareElementSolution(const KnapsackElementSolution s1, const KnapsackElementSolution s2) {
    return s1.id < s2.id;
}