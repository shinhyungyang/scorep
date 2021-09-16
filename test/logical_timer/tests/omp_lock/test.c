#include "../../tests.h"

const char* ltimer_test_name = "omp_lock";
const int   mpi_min_ranks    = 1;
const int   mpi_thread_level = MPI_THREAD_FUNNELED;

int
ltimer_test_func()
{
    omp_lock_t lock1, lock2;
    omp_init_lock( &lock1 );
    omp_init_lock( &lock2 );

    #pragma omp parallel
    {
        wait_logical_twice( omp_get_thread_num() );
        omp_set_lock( &lock1 );
        do_nothing();
        omp_unset_lock( &lock1 );
        do_nothing();
        omp_set_lock( &lock2 );
        do_nothing();
        omp_unset_lock( &lock2 );
    }
    return TEST_STATUS_SUCCESS;
}
