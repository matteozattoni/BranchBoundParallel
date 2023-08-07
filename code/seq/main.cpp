#include "../knapsack/knapsack.h"
#include <bits/stdc++.h>
#include <list>
#include <algorithm>

using namespace std;

#define SMALL_DATASET "datasets/f1_l-d_kp_10_269"      // opt 295
#define SMALL_DATASET2 "datasets/f3_l-d_kp_4_20"                 // opt 33
#define LARGE_DATASET "datasets/knapPI_1_10000_1000_1" // optimus is 563647

int main()
{
    int numberOfState = 0;
    int numberOfBranch = 0;
    int finalSolution = -1;
    std::list<KnapsackTask *> list;
    Problem *problem = readFile(SMALL_DATASET);
    KnapsackTask initTask(nullptr, 0);
    sortKnapsack(problem);
    Knapsack knapsackProblem(problem->problem, problem->nElements, problem->knapsackWeigth);
    knapsackProblem.setCurrentTask(&initTask);

    while (knapsackProblem.hasCurrentTask())
    {
        numberOfState++;
        //cout << "list size: " << list.size() << endl;
        BranchBoundResult *b = knapsackProblem.computeTaskIteration();
        switch (b->getResultType())
        {
        case Solution:
        {
            KnapsackResultSolution *solution = dynamic_cast<KnapsackResultSolution *>(b);
            int localSolution = solution->getSolutionResult();
            //cout << "solution: " << localSolution << endl;
            knapsackProblem.setBound(localSolution);
            finalSolution = max(finalSolution, localSolution);
            if (!list.empty()) {
                KnapsackTask* task = list.front();
                knapsackProblem.setCurrentTask(task);
                list.pop_front();
            }
            knapsackProblem.solutionMemoryPool.deallocate(solution);
            break;
        }
        case Branch:
        {
            KnapsackResultBranch *branch = dynamic_cast<KnapsackResultBranch *>(b);
            if (branch->getNumberBranch() > 0) {
                numberOfBranch++;
                KnapsackTask *s = (KnapsackTask *)branch->getArrayBranch();
                list.push_front(s);
            }
            knapsackProblem.branchMemoryPool.deallocate(branch);
            break;
        }
        }
    }
    cout << "Number of malloc for branch " << knapsackProblem.branchMemoryPool.numberOfMalloc << endl;
    cout << "Number of malloc for solution " << knapsackProblem.solutionMemoryPool.numberOfMalloc << endl;
    cout << "Total number of states " << numberOfState << endl;
    cout << "Total number of branch " << numberOfBranch << endl;
    cout << "Final solution is " << finalSolution << endl;
}