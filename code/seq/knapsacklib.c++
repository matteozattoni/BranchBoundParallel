
#include "knapsacklib.h"
#include <array>
#include <vector>

template<typename T, typename S, long N> void relaxed_knapsack(std::array<T,N> problem, std::vector<S> task, double W, double bound) {
    double b = W;
    double upperbound = 0;
    double fract_profit;

    for(T t: problem) {
        
    }
    elem val = nullptr;
    // get first elem
    if (val == nullptr) {  // if null there is not ammisible solution

    }

    while (b >= val->weight) {
        b -= val->weight;
        upperbound += val->profit;
        // set next elem
    }

    if (b > 0) { // relaxed solution
        fract_profit = (b / val->weight) * val->profit;
        upperbound += fract_profit;

        if (upperbound <= bound) { // close: bound

        }

        // branch

    } else { // found integer solution
        // solution = upperbound
    }


    

}

bool compareElements(const knapsackelem e1, const knapsackelem e2) {
    double val1 = (double) e1.profit / (double) e1.weight;
    double val2 = (double) e2.profit / (double) e2.weight;
    return val1 > val2; 
}