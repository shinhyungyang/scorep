#include "../../tests.h"

const char* ltimer_test_name = "omp_barrier";
const int   mpi_min_ranks    = 1;
const int   mpi_thread_level = MPI_THREAD_FUNNELED;

int
ltimer_test_func()
{
    #pragma omp parallel
    {
        wait_logical_twice( omp_get_thread_num() );
        #pragma omp barrier
    }
    return TEST_STATUS_SUCCESS;
}
