#ifndef LTIMER_TEST_TESTS_H
#define LTIMER_TEST_TESTS_H

#include "utils.h"

#include <mpi.h>
#include <omp.h>

#include <string.h>

enum TEST_STATUS
{
    TEST_STATUS_SUCCESS = 0,
    TEST_STATUS_INVALID_TEST,
    TEST_STATUS_NOT_ENOUGH_MPI_RANKS,
    TEST_STATUS_WRONG_MPI_THREAD_SUPPORT,
    TEST_STATUS_ERROR,
};

int
ltimer_test_func();

extern const int   mpi_min_ranks;
extern const char* ltimer_test_name;
extern const int   mpi_thread_level;


#endif
