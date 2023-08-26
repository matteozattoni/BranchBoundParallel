#ifndef KNAPSACKMEMORYMANAGER
#define KNAPSACKMEMORYMANAGER

#include "../branchbound/mpi/mpidatamanager.h"
#include "knapsackproblem.h"
#include "knapsackbranch.h"
#include "knapsackresult.h"

#define SMALL_DATASET1 "datasets/small/f1_l-d_kp_10_269"      // opt 295
#define SMALL_DATASET2 "datasets/small/f2_l-d_kp_20_878"       // opt 1024
#define SMALL_DATASET3 "datasets/small/f3_l-d_kp_4_20"       // opt 35
#define SMALL_DATASET4 "datasets/small/f4_l-d_kp_4_11"       // opt 23
#define SMALL_DATASET5 "datasets/small/f6_l-d_kp_10_60"       // opt 52
#define SMALL_DATASET6 "datasets/small/f7_l-d_kp_7_50"       // opt 107
#define SMALL_DATASET7 "datasets/small/f8_l-d_kp_23_10000"       // opt 9767
#define SMALL_DATASET8 "datasets/small/f9_l-d_kp_5_80"       // opt 130
#define SMALL_DATASET9 "datasets/small/f10_l-d_kp_20_879"       // opt 1025

#define MEDIUM_DATASET1 "datasets/knapPI_1_100_1000_1" // opt 9147
#define MEDIUM_DATASET2 "datasets/knapPI_2_100_1000_1" // opt 1514
#define MEDIUM_DATASET3 "datasets/knapPI_1_200_1000_1" // opt 11238

#define LARGE_DATASET "datasets/knapPI_1_10000_1000_1" // optimus is 563647
#define LARGE_DATASET2 "datasets/large/knapPI_2_10000_1000_1" // optimus is 90204
#define LARGE_DATASET3 "datasets/large/knapPI_3_10000_1000_1" // optimus is 146919


#define FILEPATH LARGE_DATASET

class KnapsackMemoryManager: public MPIDataManager
{
private:
    MPI_Datatype problemDatatype;
    MPI_Datatype problemElementDatatype;
    MPI_Datatype boundDatatype;
    MPI_Datatype branchDatatype;

    void createProblemDatatype();
    void createProblemElementDatatype();
    void createBoundDatatype();
    void createBranchDatatype();
    
public:
KnapsackMemoryManager(/* args */);
    ~KnapsackMemoryManager();
    void testProblemMemory();
    void testBranchMemory();
    void testBoundMemory();
    void test2();

    Branch* getRootBranch() override;

    // MPIDataManager methods
    MPI_Datatype getProblemType() override;
    MPI_Datatype getProblemElementType() override;
    MPI_Datatype getBoundType() override;
    MPI_Datatype getBranchType() override;
    void commitDatatypes() override;

    // Branch and Bound Problem
    // master (0)
    BranchBoundProblem* getLocalProblem() const override;
    void* getProblemTypeBuffFrom(BranchBoundProblem* problem) override;
    std::pair<void*,int> getProblemElementBuffFrom(BranchBoundProblem* problem) override;

    // slaves
    void* getEmptyProblemTypeBuff() override;
    std::pair<void*,int> getEmptyProblemElementBuffFromType(void* problemType) override;
    BranchBoundProblem* getRemoteProblem(void* problemType, std::pair<void*,int> problemElements) override;

    // BOUND
    std::pair<void*,int> getBoundBuffer(const BranchBoundResultSolution* solution) override;
    void* getEmptybBoundBuff() override;
    BranchBoundResultSolution* getSolutionFromBound(void* buff) override;
    
    // RECV
    void* getEmptyBranchElementBuff(int count) override;
    
    // SEND
    std::pair<void*,int> getBranchBuffer(const Branch* branch) override;
    void sentFinished(void* buff, int count) override;
    BranchBoundResultBranch* getBranchFromBuff(void* buff, int count) override;
};

#endif