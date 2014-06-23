#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */
#include <time.h>

// define global varaibles
static int  NUM_OF_THREADS = 5;
static long NUM_OF_ITER    = 100;
static long CNT            = 0;
// initialize mutex and cond with default values
static pthread_mutex_t LOCK          = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  COND          = PTHREAD_COND_INITIALIZER;
static int             CONDITION     = 0;
static long            WAIT_TIME_SEC = 5;

void
check_status( char* string, int status )
{
    if ( status )
    {
        printf( "%s test FAILED with error %d.\n", string, status );
        exit( 1 );
    }
#ifdef DEBUG
    printf( "%s test PASSED\n", string );
#endif
}

void*
test_mutex( void* input )
{
    long tid, cnt = 0;
    // get thread id
    tid = ( long )input;
    int i, j, k, status;

    // make one iteration
    for ( i = 0; i < 1; i++ )
    {
        status = pthread_mutex_trylock( &LOCK );
        if ( status == EBUSY )
        {
            cnt++;
            continue;
        }
        check_status( "pthread_mutex_trylock()", status );
        CNT++;
        status = pthread_mutex_unlock( &LOCK );
        check_status( "pthread_mutex_unlock()", status );
    }

    // lock the thread
    status = pthread_mutex_lock( &LOCK );
    check_status( "pthread_mutex_lock()", status );
    CNT += cnt;
    // unlock the thread
    status = pthread_mutex_unlock( &LOCK );
    check_status( "pthread_mutex_unlock()", status );
}

void*
test_cond( void* input )
{
    long tid;
    tid = ( long )input;

    int status;

    status = pthread_mutex_lock( &LOCK );
    check_status( "pthread_mutex_lock()\n", status );

    while ( !CONDITION )
    {
        printf( "Thread %ld is blocked\n", tid );
        status = pthread_cond_wait( &COND, &LOCK );
        check_status( "pthread_cond_wait()\n", status );
    }

    printf( "Thread %ld is awoken!\n", tid );

    status = pthread_mutex_unlock( &LOCK );
    check_status( "pthread_mutex_lock()\n", status );
}

void*
test_cond_timedwait( void* input )
{
    long tid;
    tid = ( long )input;
    struct timespec ts;

    int status;

    status = pthread_mutex_lock( &LOCK );
    check_status( "pthread_mutex_lock()\n", status );
    while ( 1 )
    {
        ts.tv_sec  = time( NULL ) + WAIT_TIME_SEC;
        ts.tv_nsec = 0;

        while ( !CONDITION )
        {
            printf( "Thread %ld is blocked\n", tid );
            status = pthread_cond_timedwait( &COND, &LOCK, &ts );
            if ( status == ETIMEDOUT )
            {
                printf( "Thread %ld time is out!\n", tid );
                status = pthread_mutex_unlock( &LOCK );
                check_status( "pthread_mutex_unlock()\n", status );
                return NULL;
            }
            check_status( "pthread_cond_timedwait()\n", status );
        }
        printf( "Thread %ld is awoken!\n", tid );
        CONDITION = 0;
    }
    status = pthread_mutex_unlock( &LOCK );
    check_status( "pthread_mutex_lock()\n", status );
}

int
main( int argc, char* argv[] )
{
    int       status;
    long      i;
    pthread_t t[ NUM_OF_THREADS ];

    printf( "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
    printf( "+++++++++++++ Test of synchronization routines +++++++++++++\n" );
    printf( "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
    printf( "\n+++++++++++++++++++++++ Mutex tests ++++++++++++++++++++++++\n" );

    status = pthread_mutex_init( &LOCK, NULL );
    check_status( "pthread_mutex_init()", status );

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_create( &t[ i ], NULL, test_mutex, ( void* )i );
        check_status( "pthread_create()", status );
    }

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_join( t[ i ], NULL );
        check_status( "pthread_join()", status );
    }

    status = pthread_mutex_destroy( &LOCK );
    check_status( "pthread_mutex_destroy()", status );

    if ( CNT == NUM_OF_THREADS )
    {
        printf( "\nMutex test PASSED CNT = %ld is equal number of threads (i.e. %d)\n",
                CNT, NUM_OF_THREADS );
    }
    else
    {
        printf( "\nMutex test FAILED CNT = %ld is NOT equal number of threads (i.e. %d)\n",
                CNT, NUM_OF_THREADS );
        exit( 1 );
    }

    printf( "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
    printf( "++++++++++++++++++ Conditional mutex tests ++++++++++++++++++\n" );

    pthread_condattr_t attr;
    status = pthread_condattr_init( &attr );
    check_status( "pthread_condattr_init()", status );

    status = pthread_cond_init( &COND, &attr );
    check_status( "pthread_cond_init()", status );

    status = pthread_mutex_init( &LOCK, NULL );
    check_status( "pthread_mutex_init()", status );

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_create( &t[ i ], NULL, test_cond, ( void* )i );
        check_status( "pthread_create()", status );
    }

    // make pause to show that all threads are sleeping
    sleep( 1 );
    // change condition
    CONDITION = 1;

    // wake thread one by one
    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_cond_signal( &COND );
        check_status( "pthread_cond_signal()\n", status );
        sleep( 1 );
    }

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_join( t[ i ], NULL );
        check_status( "pthread_join()", status );
    }

    // reset condition
    CONDITION = 0;

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_create( &t[ i ], NULL, test_cond, ( void* )i );
        check_status( "pthread_create()", status );
    }

    // make pause to show that all threads are sleeping
    sleep( 1 );
    // change condition
    CONDITION = 1;

    // wake all threads at once
    status = pthread_cond_broadcast( &COND );
    check_status( "pthread_cond_broadcast()\n", status );

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_join( t[ i ], NULL );
        check_status( "pthread_join()", status );
    }

    // reset condition
    CONDITION = 0;
    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_create( &t[ i ], NULL, test_cond_timedwait, ( void* )i );
        check_status( "pthread_create()", status );
    }

    // make pause to show that all threads are sleeping
    sleep( 2 );
    // change condition
    CONDITION = 1;

    // wake up one thread
    status = pthread_cond_signal( &COND );

    for ( i = 0; i < NUM_OF_THREADS; i++ )
    {
        status = pthread_join( t[ i ], NULL );
        check_status( "pthread_join()", status );
    }

    status = pthread_condattr_destroy( &attr );
    check_status( "pthread_condattr_destroy()", status );

    status = pthread_cond_destroy( &COND );
    check_status( "pthread_cond_destroy()", status );

    status = pthread_mutex_destroy( &LOCK );
    check_status( "pthread_mutex_destroy()", status );

    printf( "Test PASSED\n" );

    return 0;
}
