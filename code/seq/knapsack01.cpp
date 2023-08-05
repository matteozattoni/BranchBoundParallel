#include "knapsackclass.h"
#include "knapsacklib.h"

void Knapsack01::setBound(int bound)
{
    this->bound = bound;
}

void Knapsack01::readFile(std::string fileName)
{
    char line[LINE_DIMENSION];
    const char *delimeter = " ";
    char *val;
    ifstream file(fileName);

    // Get Problem Dimension and Knapsack Total Capacity
    file.getline(line, sizeof(line));
    val = strtok(line, delimeter);
    problemDimension = atoi(val);
    problem.elements = (knapsackelem *)calloc(problemDimension, sizeof(struct knapsackelem));

    cout << problemDimension << endl;
    val = strtok(NULL, delimeter);
    knapsackCapacity = atoi(val);
    cout << knapsackCapacity << endl;

    // get istance
    for (size_t i = 0; i < problemDimension; i++)
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
            problem.elements[i].profit = profit;
            problem.elements[i].weight = weight;
        }
    }

    cout << "first " << problem.elements[0].profit << endl;                   // 94
    cout << "last " << problem.elements[problemDimension - 1].profit << endl; // 455

    file.close();
}

void Knapsack01::sortKnapsack()
{
    sort(problem.elements, problem.elements + problemDimension, compareElements);
    cout << "first " << problem.elements[0].profit << endl; // 94
    cout << "last " << problem.elements[problemDimension - 1].profit << endl;
    cout << "size of class is: " << sizeof(problem.elements[0]) * problemDimension << endl;
    for (size_t i = 0; i < problemDimension - 1; i++)
    {
        const knapsackelem e1 = problem.elements[i];
        const knapsackelem e2 = problem.elements[i + 1];
        double val1 = (double)e1.profit / (double)e1.weight;
        double val2 = (double)e2.profit / (double)e2.weight;
        if (!compareElements(e1, e2))
        {
            if (val1 != val2)
            {
                cout << "ERROR e1: " << val1 << " e2: " << val2 << endl;
                throw;
            }
        }
    }
}


Knapsack01::Knapsack01() {
}

Knapsack01::~Knapsack01() {
}
