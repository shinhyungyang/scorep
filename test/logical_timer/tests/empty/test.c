#include "../../tests.h"

const char* ltimer_test_name = "empty";
const int   mpi_min_ranks    = 1;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    return TEST_STATUS_SUCCESS;
}
