#include <iostream>
#include <list>
#include <algorithm>
#include "../knapsack/knapsack.h"


using namespace std;

#define SMALL_DATASET "datasets/f1_l-d_kp_10_269"      // opt 295
#define SMALL_DATASET2 "datasets/f3_l-d_kp_4_20"                 // opt 33
#define LARGE_DATASET "datasets/knapPI_1_10000_1000_1" // optimus is 563647

int main()
{
    KnapsackMemoryManager* manager = KnapsackMemoryManager::singleton;
    int numberOfState = 0;
    int numberOfBranch = 0;
    int finalSolution = -1;
    std::list<KnapsackTask *>* list = new std::list<KnapsackTask*>();
    Problem *problem = readFile(SMALL_DATASET);
    KnapsackTask *initTask = manager->allocateTask();
    new(initTask) KnapsackTask(nullptr, 0);
    sortKnapsack(problem);
    Knapsack knapsackProblem(problem->problem, problem->nElements, problem->knapsackWeigth);

    knapsackProblem.setCurrentTask(initTask);
    while (knapsackProblem.hasCurrentTask())
    {
        numberOfState++;
        BranchBoundResult *b = knapsackProblem.computeTaskIteration();
        switch (b->getResultType())
        {
        case Solution:
        {
            
            KnapsackResultSolution *solution = dynamic_cast<KnapsackResultSolution *>(b);
            int localSolution = solution->getSolutionResult();
            knapsackProblem.setBound(localSolution);
            finalSolution = max(finalSolution, localSolution);
            if (!list->empty()) {
                KnapsackTask* task = list->front();
                knapsackProblem.setCurrentTask(task);
                if (task != nullptr && task->objects != nullptr)
                    manager->deallocateArray(task->objects);
                manager->deallocateTask(task);
                list->pop_front();
            }
            manager->deallocateResultSolution(solution);
            break;
        }
        case Branch:
        {
            KnapsackResultBranch *branch = dynamic_cast<KnapsackResultBranch *>(b);
            if (branch->getNumberBranch() > 0) {
                numberOfBranch++;
                KnapsackTask *s = (KnapsackTask *)branch->getArrayBranch();
                list->push_front(s);
            }
            manager->deallocateResultBranch(branch);
            break;
        }
        }
    }
    
    cout << "Number of calloc for task " << manager->getNumberArrayCalloc() << endl;
    cout << "Number of malloc for branch " << manager->getNumberResultBranchMalloc() << endl;
    cout << "Number of malloc for solution " << manager->getNumberResultSolutionMalloc() << endl;
    cout << "Number of malloc for task " << manager->getNumberTaskMalloc() << endl;
    cout << "Total number of states " << numberOfState << endl;
    cout << "Total number of branch " << numberOfBranch << endl;
    cout << "Final solution is " << finalSolution << endl;
}