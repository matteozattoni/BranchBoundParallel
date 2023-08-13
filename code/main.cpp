#include <iostream>
#include <list>
#include <algorithm>
#include <chrono>
#include "knapsack/knapsack.h"
#include "branchbound/branchbound.h"

using namespace std;

#define SMALL_DATASET1 "datasets/small/f1_l-d_kp_10_269"      // opt 295
#define SMALL_DATASET2 "datasets/small/f2_l-d_kp_20_878"       // opt 1024
#define SMALL_DATASET3 "datasets/small/f3_l-d_kp_4_20"       // opt 35
#define SMALL_DATASET4 "datasets/small/f4_l-d_kp_4_11"       // opt 23
#define SMALL_DATASET5 "datasets/small/f6_l-d_kp_10_60"       // opt 52
#define SMALL_DATASET6 "datasets/small/f7_l-d_kp_7_50"       // opt 107
#define SMALL_DATASET7 "datasets/small/f8_l-d_kp_23_10000"       // opt 9767
#define SMALL_DATASET8 "datasets/small/f9_l-d_kp_5_80"       // opt 130
#define SMALL_DATASET9 "datasets/small/f10_l-d_kp_20_879"       // opt 1025


#define MEDIUM_DATASET "datasets/knapPI_1_100_1000_1" // opt 9147
#define MEDIUM_DATASET2 "datasets/knapPI_1_200_1000_1" // opt 11238
#define LARGE_DATASET "datasets/knapPI_1_10000_1000_1" // optimus is 563647

int main()
{
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::seconds sec;
    typedef std::chrono::duration<float> fsec;
    auto t0 = Time::now();
    Knapsack *knapsack = new Knapsack();
    BranchBound *branchBound = new BranchBound(knapsack);
    KnapsackProblem *problem = KnapsackProblem::problemFromFile(SMALL_DATASET7);
    cout << "Start Branch and Bound" << endl;
    try
    {
        branchBound->start(problem, true);
        //KnapsackMemoryRecap *memoryRecap = manager->getMemoryInfo();
        //cout << *memoryRecap << endl;
        // cout << "Total number of states " << numberOfState << endl;
        // cout << "Total number of branch " << numberOfBranch << endl;
        cout << *branchBound << endl;
        cout << "Final solution is " << branchBound->bound << endl;
        auto t1 = Time::now();
        fsec fs = t1 - t0;
        sec d = std::chrono::duration_cast<sec>(fs);
        cout << "Total duration: " << d.count() << "s" << endl;
        //delete memoryRecap;
        return 0;
    }
    catch (eBranchBoundException e)
    {
        cout << "Exception n: " << e << endl;
        return 1;
    }
}