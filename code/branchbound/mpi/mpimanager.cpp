#include "mpimanager.h"
#include <iostream>
#include <cmath>


using namespace std;


MPIManager::MPIManager(MPIDataManager* manager): dataManager(*manager)
{
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    const int myworkpool = worldRank / WORKPOOL_N;
    MPI_Comm_split(MPI_COMM_WORLD, myworkpool, worldRank, &workpoolComm);
    MPI_Comm_size(workpoolComm, &workpoolSize);
    MPI_Comm_rank(workpoolComm, &workpoolRank);
    receiveBranch[workpoolRank].ready = false;
    sentBranch[workpoolRank].isSent = true;
    manager->commitDatatypes();
    receiveBound.boundBuffer = manager->getEmptybBoundBuff();
    MPI_Irecv(receiveBound.boundBuffer, 1, manager->getBoundType(), MPI_ANY_SOURCE, tagMessage.bound, workpoolComm, &receiveBound.request);
}

MPIManager::~MPIManager()
{
    MPI_Finalize();
}

const Branch* MPIManager::getRootBranch(){
    if (worldRank == 0)
        return dataManager.getRootBranch();
    else
        return nullptr;
}

void MPIManager::init() {
    
}

BranchBoundProblem* MPIManager::getBranchProblem() {
    MPI_Datatype problemType = dataManager.getProblemType();
    MPI_Datatype problemElementType = dataManager.getProblemElementType();
    void* problemDescritpionBuffer;
    std::pair<void*,int> problemElementsPair;
    if (worldRank == MASTER_RANK) {
        BranchBoundProblem* localProblem = dataManager.getLocalProblem();
        problemDescritpionBuffer = dataManager.getProblemTypeBuffFrom(localProblem);
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getProblemElementBuffFrom(localProblem);
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, MASTER_RANK, MPI_COMM_WORLD);
        return localProblem; // could be memory leak
    } else {
        problemDescritpionBuffer = dataManager.getEmptyProblemTypeBuff();
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getEmptyProblemElementBuffFromType(problemDescritpionBuffer);
        MPI_Status status;
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, MASTER_RANK, MPI_COMM_WORLD);
        BranchBoundProblem* remoteProblem = dataManager.getRemoteProblem(problemDescritpionBuffer, problemElementsPair);
        return remoteProblem; // could be memory leak
    }
}


void MPIManager::prologue(std::function<void(BranchBoundResult*)> callback) {
    int boundFlag;
    MPI_Status status;

    // bound message??? mpi test and probe for bound
    MPI_Test(&receiveBound.request, &boundFlag, &status);
    if (boundFlag == true) {
        BranchBoundResultSolution* sol = dataManager.getSolutionFromBound(receiveBound.boundBuffer);
        callback(sol);
        receiveBound.boundBuffer = nullptr;
        // there are more?
        while (boundFlag)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, tagMessage.bound, workpoolComm, &boundFlag, &status);
            if (boundFlag) {
                // get buffer for bound
                receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
                MPI_Recv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, &status);
                BranchBoundResultSolution* sol = dataManager.getSolutionFromBound(receiveBound.boundBuffer);
                callback(sol);
                receiveBound.boundBuffer = nullptr;
            }
        }
        receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
        MPI_Irecv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), MPI_ANY_SOURCE, tagMessage.bound, workpoolComm, &receiveBound.request);
    }

    // recv branch
    loadBalance(callback);
}

void MPIManager::loadBalance(std::function<void(BranchBoundResult*)> callback) {
    for (int i = 0; i < workpoolSize; i++) {
        MPI_Status status;
        int hasSent;
        if (receiveBranch[i].ready == false && i != workpoolRank) {
            MPI_Iprobe(i, tagMessage.branch, workpoolComm, &hasSent, &status);
            if (hasSent) {
                int count;
                MPI_Get_count(&status, dataManager.getBranchType(), &count);
                void* buffBranch = dataManager.getEmptyBranchElementBuff(count);
                MPI_Irecv(buffBranch, count, dataManager.getBranchType(), i, tagMessage.branch, workpoolComm, &receiveBranch[i].request);
                receiveBranch[i].ready = true;
                receiveBranch[i].count = count;
                receiveBranch[i].branchBuffer = buffBranch;
            }
        }
    }    
}

void MPIManager::epilogue(std::function<const Branch*()> callback) {
    for (int i = 0; i < workpoolSize; i++) {
        MPI_Status status;
        int hasSent, testFlag;
        if (i != workpoolRank && sentBranch[i].isSent == false) {
            const Branch* branch = callback();
            if (branch != nullptr) { // sent
                std::pair<void*,int> pair = dataManager.getBranchBuffer(branch);
                MPI_Issend(pair.first, pair.second, dataManager.getBranchType(), i, tagMessage.branch, workpoolComm, &sentBranch[i].request);
                sentBranch[i].isSent = true;
                sentBranch[i].sentBufferAndCount = pair;
            } else {
                break;
            }
        } else if (i != workpoolRank) {
            MPI_Test(&sentBranch[i].request, &testFlag, &status);
            if (testFlag) {
                dataManager.sentFinished(sentBranch[i].sentBufferAndCount.first, sentBranch[i].sentBufferAndCount.second);
                sentBranch[i].isSent = false;
                const Branch* branch = callback();
                if (branch != nullptr) {
                    std::pair<void*,int> pair = dataManager.getBranchBuffer(branch);
                    MPI_Issend(pair.first, pair.second, dataManager.getBranchType(), i, tagMessage.branch, workpoolComm, &sentBranch[i].request);
                    sentBranch[i].isSent = true;
                    sentBranch[i].sentBufferAndCount = pair;
                } else {
                    break;
                }
            }
        }
    }
}

BranchBoundResultBranch* MPIManager::waitForBranch() {
    for(int i = 0; i< workpoolSize; i++) { // receive already commited (see if completed)
        if (i != workpoolRank && receiveBranch[i].ready == true) {
            MPI_Status status;
            int isCompleted;
            MPI_Test(&receiveBranch[i].request, &isCompleted, &status);
            if(isCompleted) { // here a recevice is commited AND completed
                BranchBoundResultBranch* branch = dataManager.getBranchFromBuff(receiveBranch[i].branchBuffer, receiveBranch[i].count);
                receiveBranch[i].ready = false;
                return branch;
            }
        }
    }
    MPI_Request arrayRequest[workpoolSize];
    int nRecvReady = 0;
    for (int i = 0; i < workpoolSize; i++)  // take the request of all receive commited
    {
        if (receiveBranch[i].ready) {
            arrayRequest[nRecvReady++] = receiveBranch[i].request;
        }
    }
    
    MPI_Status status;
    int index;
    if (nRecvReady > 0) { // if there is come receive commited i'll wait for it
        MPI_Waitany(nRecvReady, arrayRequest, &index, &status);
        int count;
        BranchBoundResultBranch* branch = dataManager.getBranchFromBuff(receiveBranch[index].branchBuffer, receiveBranch[index].count);
        receiveBranch[index].ready = false;
        return branch;
    } else { // all receive are not commited so i'll wait the first message from other (termination)
        MPI_Probe(MPI_ANY_SOURCE, tagMessage.branch, workpoolComm, &status);
        int count;
        MPI_Get_count(&status, dataManager.getBranchType(), &count);
        void* buffBranch = dataManager.getEmptyBranchElementBuff(count);
        MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, &status);
        BranchBoundResultBranch* branch = dataManager.getBranchFromBuff(buffBranch, count);
        return branch;
    }
    
}

void MPIManager::defineType(){
    struct
    {
        int a;
        double b;
        double c;
    } myss;
    
    int size;
    MPI_Aint lb, ext;
    MPI_Datatype mystruct;
    
    int arrayOfBlocklenghs[] = {1,2};
    MPI_Aint arrayOfDisplacements[2];
    MPI_Datatype arrayOfOldDatatype[] = {MPI_INT, MPI_DOUBLE};
    MPI_Get_address(&myss.a, &arrayOfDisplacements[0]);
    MPI_Get_address(&myss.b, &arrayOfDisplacements[1]);
    arrayOfDisplacements[1] -= arrayOfDisplacements[0];
    cout << "disp is " << arrayOfDisplacements[1] << endl;
    arrayOfDisplacements[0] = 0;
    MPI_Type_create_struct(2, arrayOfBlocklenghs, arrayOfDisplacements, arrayOfOldDatatype, &mystruct);
    MPI_Type_commit( &mystruct );
    MPI_Type_size(mystruct, &size);
    MPI_Type_get_extent(mystruct, &lb, &ext);
    cout << "mpi size is " << size << " lb: " << lb << " ext: "<<ext<< endl;
    cout << "struct size is " << sizeof(myss) << endl;
    
}