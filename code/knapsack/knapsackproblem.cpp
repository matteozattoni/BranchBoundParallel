
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include "knapsackproblem.h"

using namespace std;

KnapsackProblem::KnapsackProblem(BranchBoundProblemElement* problemElements, int numberElements, int knapsackCap) : BranchBoundProblem(problemElements, numberElements), knapsackCapacity(knapsackCap) {}

KnapsackProblem::~KnapsackProblem() {}

KnapsackProblemElement::KnapsackProblemElement(double p, double w): profit(p), weight(w) {}

const KnapsackProblemElement* KnapsackProblem::getKnapsackProblemElements() const {
    const BranchBoundProblemElement* elements = this->getProblemElements();
    const KnapsackProblemElement* knapsackElements = dynamic_cast<const KnapsackProblemElement*>(elements);
    if (knapsackElements == nullptr)
        throw InvalidProblemElementCast;
    return knapsackElements;
}

int KnapsackProblem::getTotalKnapsackCapacity() const {
    return knapsackCapacity;
}

KnapsackProblem* KnapsackProblem::problemFromFile(std::string path)
{

    //Problem* problem = (Problem*) malloc(sizeof(Problem));
    char line[LINE_DIMENSION];
    const char *delimeter = " ";
    char *val;
    
    ifstream file(path);

    // Get Problem Dimension and Knapsack Total Capacity
    file.getline(line, sizeof(line));
    val = strtok(line, delimeter);


    const int problemDimension = atoi(val);
    KnapsackProblemElement* elements = (KnapsackProblemElement *) calloc(problemDimension, sizeof(KnapsackProblemElement));
    val = strtok(NULL, delimeter);
    const int knapsackTotalWeight = atoi(val);

    // get istance
    for (int i = 0; i < problemDimension; i++)
    {
        char line[LINE_DIMENSION];
        char *val;
        
        if (!file.eof())
        {
            file.getline(line, sizeof(line));
            val = strtok(line, delimeter);
            const double profit = atoi(val);
            val = strtok(NULL, delimeter);
            const double weight = atoi(val);
            new(&elements[i]) KnapsackProblemElement(profit, weight);
        }
    }
    struct cmp {
        bool operator()(const KnapsackProblemElement e1, const KnapsackProblemElement e2) {
            double val1 = (double) e1.profit / (double) e1.weight;
            double val2 = (double) e2.profit / (double) e2.weight;
            return val1 > val2; 
        }
    } comparator;
    sort(elements, elements + problemDimension, comparator);
    file.close();
    return new KnapsackProblem(elements, problemDimension, knapsackTotalWeight);
}