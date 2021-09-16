#include "../../tests.h"

/**
 * Score-P produces a warning if an MPI application
 * requests a thread support greater than MPI_THREAD_FUNNELED.
 *
 * The point of this test is to capture this warning and expect to fail.
 *
 * If at some point in the future Score-P works with MPI_THREAD_SERIALIZED
 * or higher, the test code will succeed without warning and
 * the test suite will notify us that the logical timer module needs to be
 * adapted for the higher level of thread support.
 */
const char* ltimer_test_name = "mpi_thread_serialized";
const int   mpi_min_ranks    = 1;
const int   mpi_thread_level = MPI_THREAD_SERIALIZED;

int
ltimer_test_func()
{
    return TEST_STATUS_SUCCESS;
}
