#ifndef KNAPSACKPROBLEM_H
#define KNAPSACKPROBLEM_H

#include <string>
#include "../branchbound/branchboundproblem.h"
#include "../branchbound/branchboundexception.h"

class KnapsackProblemElement: public BranchBoundProblemElement {
public:
    double profit;
    double weight;
    KnapsackProblemElement(double profit, double weigth);
};




class KnapsackProblem : public BranchBoundProblem
{
private:
    const int knapsackCapacity;
    static const int LINE_DIMENSION=50;
    /* data */
public:
    const KnapsackProblemElement* getKnapsackProblemElements() const;
    int getTotalKnapsackCapacity() const;
    KnapsackProblem(BranchBoundProblemElement* elements, int numberElements, int knapsackCapacity);
    ~KnapsackProblem();
    static KnapsackProblem* problemFromFile(std::string path);
    
};



#endif