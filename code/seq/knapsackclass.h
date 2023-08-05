#ifndef KNAPSACKCLASS
#define KNAPSACKCLASS

#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "knapsacklib.h"

#include <unistd.h>
#include <string.h>

#define LINE_DIMENSION 50

using namespace std;

class Knapsack01 {

private:
    int bound;
    int problemDimension;
    int knapsackCapacity;
    knapsackproblem problem;

    
public:
    Knapsack01();
    ~Knapsack01();

    void setBound(int bound);

    void readFile(std::string fileName);

    void sortKnapsack();

};

#endif