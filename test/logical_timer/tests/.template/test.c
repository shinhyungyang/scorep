#include "../../tests.h"

/**
 * Set this to the name of the containing directory
 */
const char* ltimer_test_name = "template";

/**
 * The minimum number of MPI ranks that this test needs to run successfully
 */
const int mpi_min_ranks = 2;

/**
 * The level of thread support for MPI
 */
const int mpi_thread_level = MPI_THREAD_FUNNELED;

/**
 * This function will be called by the driver
 *
 * Put the code that should be instrumented and traced here.
 */
int
ltimer_test_func()
{
    return TEST_STATUS_SUCCESS;
}
