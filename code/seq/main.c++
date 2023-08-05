#include "knapsackclass.h"

int main() {
    
    Knapsack01 knapsack;
    knapsack.readFile("datasets/knapPI_1_10000_1000_1");
    knapsack.sortKnapsack();
}