// OpenMP program to print Hello World
// using C language

// OpenMP header
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>


/* This dummy function exists because printf alone is not insturemented.*/
void
print_string( char str[] )
{
    printf( "%s\n", str );
}

void
foo( int thread_num )
{
    int sum;

    for ( int i = 0; i < 10; i++ )
    {
        sum += i;
    }
}

/******************************************************************************/
/* Goal: One of children threads has to have higher load than master */
void
test_fork_join()
{
    int count = 0;

    #pragma omp parallel num_threads(4)
    {
        for ( int i = 0; i < omp_get_max_threads(); i++ )
        {
            count++;
            if ( count == 10 )
            {
                for ( int y = 0; y < 20; y++ )
                {
                    print_string( "Inside count=10 \n" );
                }
            }
        }
    }
}

/******************************************************************************/
/* Goal: Different loads between threads, after barrier all are the same */
void
test_barrier()
{
    int count = 0;

    print_string( "pre parllel" );

    #pragma omp parallel num_threads(4)
    {
        for ( int i = 0; i < omp_get_max_threads(); i++ )
        {
            count++;
            if ( count == 10 )
            {
                foo( omp_get_thread_num() );
            }
        }
        #pragma omp barrier
    }
}

/******************************************************************************/
/*Goal:  */
void
test_critical()
{
    #pragma omp parallel num_threads(4)
    {
        foo( omp_get_thread_num() );
        #pragma omp critical
        print_string( "inside critical section.\n" );
    }
}

/******************************************************************************/
/*Goal:  */
void
test_lock()
{
    omp_lock_t lock;
    omp_init_lock( &lock );

    #pragma omp parallel num_threads(4)
    {
        foo( omp_get_thread_num() );
        omp_set_lock( &lock );
        print_string( "acquired lock.\n" );
        omp_unset_lock( &lock );
    }
}

/******************************************************************************/
/*Goal:  */
void
test_multi_lock()
{
    omp_lock_t lock, lock2;
    omp_init_lock( &lock );
    omp_init_lock( &lock2 );

    #pragma omp parallel num_threads(4)
    {
        foo( omp_get_thread_num() );
        omp_set_lock( &lock );
        print_string( "Lock 1 acquired\n" );
        omp_unset_lock( &lock );
        print_string( "Lock 1 released\n" );
        omp_set_lock( &lock2 );
        print_string( "Lock 2 acquired\n" );
        omp_unset_lock( &lock2 );
        print_string( "Lock 2 released\n" );
    }
}

/******************************************************************************/
void
test_combined()
{
    #pragma omp parallel
    {
        printf( "Hello World... from thread = %d\n", omp_get_thread_num() );
        #pragma omp master
        {
            printf( "Hello World... from thread = %d inside master\n", omp_get_thread_num() );
        }
        #pragma omp barrier
        printf( "Hello World... from thread = %d after explicit barrier\n", omp_get_thread_num() );
        #pragma omp master
        {
            printf( "Hello World... from thread = %d inside master\n", omp_get_thread_num() );
        }
    }
}

/******************************************************************************/
void
test_implicit_barrier()
{
    int count = 0;

    print_string( "pre parllel" );

    #pragma omp parallel num_threads(4)
    {
        for ( int i = 0; i < omp_get_max_threads(); i++ )
        {
            count++;
            if ( count == 10 )
            {
                foo( omp_get_thread_num() );
            }
        }
    }
}

/******************************************************************************/
int
main( int argc, char* argv[] )
{
    test_fork_join();
    test_barrier();
    test_critical();
    test_lock();
    test_multi_lock();
    test_combined();
    test_implicit_barrier();

    return 0;
}
