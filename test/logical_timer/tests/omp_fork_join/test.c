#include "../../tests.h"

const char* ltimer_test_name = "omp_fork_join";
const int   mpi_min_ranks    = 1;
const int   mpi_thread_level = MPI_THREAD_FUNNELED;

int
ltimer_test_func()
{
    #pragma omp parallel
    {
        for ( int i = 0; i <  ( omp_get_thread_num() * 2 ); i++ )
        {
            /* only one child thread executes this section */
            if ( i ==  omp_get_thread_num() )
            {
                do_nothing();
            }
        }
    }
    /* another thread fork event */
    #pragma omp parallel
    {
        for ( int i = 0; i <  ( omp_get_thread_num() * 2 ); i++ )
        {
            /* only one child thread executes this section */
            if ( i ==  omp_get_thread_num() )
            {
                do_nothing();
            }
        }
    }
    return TEST_STATUS_SUCCESS;
}
