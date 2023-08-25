#ifndef MPIEXCEPTIONS_H
#define MPIEXCEPTIONS_H

#include <exception>
#include <string>

#define WORKPOOL_WORKER 32

class MPIGeneralException : public std::exception {
private:
    const std::string reason;
public:
    MPIGeneralException(std::string reasonString) : reason(reasonString) {};
    const char * what () const throw () {
        return reason.c_str();
    }
};

class MPIUnimplementedException : public std::exception {
private:
    const std::string reason;
public:
    MPIUnimplementedException(std::string reasonString) : reason(reasonString) {};
    const char * what () const throw () {
        return reason.c_str();
    }
};


class MPILocalTerminationException: public std::exception
{
private:
public:
    MPILocalTerminationException() {};
    ~MPILocalTerminationException() {};
    const char * what () const throw () {
        return "MPILocalTerminationException";
    }
};


class MPIWorkpoolTerminationException: public std::exception
{
private:
public:
    MPIWorkpoolTerminationException() {};
    ~MPIWorkpoolTerminationException() {};
    const char * what () const throw () {
        return "MPIWorkpoolTerminationException";
    }
};

class MPIGlobalTerminationException: public std::exception
{
private:
public:
    MPIGlobalTerminationException() {};
    ~MPIGlobalTerminationException() {};
    const char * what () const throw () {
        return "MPIGlobalTerminationException";
    }
};

class MPIBranchBoundTerminationException: public std::exception {
private:
public:
double finalSolution;
MPIBranchBoundTerminationException(double finalSolution) {this->finalSolution = finalSolution;};
~MPIBranchBoundTerminationException() {};
const char * what () const throw () {
        return "MPIBranchBoundTerminationException";
    }
};


#endif