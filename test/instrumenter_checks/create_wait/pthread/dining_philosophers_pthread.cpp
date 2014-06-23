#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define EATPOLITE

typedef struct philosoph
{
    double think_time;
    double eat_time;
    int    left_fork;
    int    right_fork;
    long   thread_id;
#ifdef EATPOLITE
    long   left_neighb;
    long   right_neighb;
#endif
    time_t total_thinking_time;
    time_t total_eating_time;
    time_t total_starvation_time;
    long   spagetti;
} philosoph_t;

const static long NUM_PHILOSOPHERS = 5;
const static long SPAGETTI         = 10;

static pthread_mutex_t forks[ 5 ];
static philosoph_t     ph[ NUM_PHILOSOPHERS ];

time_t
think( double think_time, long thread_id )
{
    time_t timer = time( NULL );
#ifdef DEBUG
    printf( "Philosopher %ld is thinking\n", thread_id );
#endif
    usleep( think_time );
    return time( NULL ) - timer;
}

time_t
eat( double eat_time, long thread_id, long i )
{
    time_t timer = time( NULL );
#ifdef DEBUG
    printf( "Philosopher %ld is eating %ld out of %ld\n",
            thread_id, SPAGETTI - i, SPAGETTI );
#endif
    usleep( eat_time );
    return time( NULL ) - timer;
}

void*
dining( void* input )
{
    time_t       timer = 0;
    philosoph_t* ph    = ( philosoph_t* )input;
    srand( time( NULL ) + ph->thread_id );

    ph->think_time = 1000000 * ( double )rand() / ( double )RAND_MAX;
    ph->eat_time   = 1000000 * ( double )rand() / ( double )RAND_MAX;

#ifdef DEBUG
    printf( "I'm philosopher #%ld my left fork is #%ld my right fork is #%ld my "
            "thinking time is %g ms my eating time is %g ms\n", ph->thread_id,
            ph->left_fork, ph->right_fork, ph->think_time, ph->eat_time );
#endif
    while ( ph->spagetti != 0 )
    {
        pthread_mutex_lock( &forks[ ph->left_fork ] );
#ifdef EATPOLITE
        if ( pthread_mutex_trylock( &forks[ ph->right_fork ] ) != 0 &&
             ( ph->spagetti <= ph[ ph->left_neighb ].spagetti ||
               ph->spagetti <= ph[ ph->right_neighb ].spagetti ) )
#else
        if ( pthread_mutex_trylock( &forks[ ph->right_fork ] ) != 0 )
#endif
        {
            timer = time( NULL );
            pthread_mutex_unlock( &forks[ ph->left_fork ] );
            continue;
        }

        if ( time( NULL ) > timer && timer != 0 )
        {
            ph->total_starvation_time += time( NULL ) - timer;
            timer                      = 0;
        }
        ph->spagetti           = ph->spagetti - 1;
        ph->total_eating_time += eat( ph->eat_time, ph->thread_id, ph->spagetti );
        pthread_mutex_unlock( &forks[ ph->right_fork ] );
        pthread_mutex_unlock( &forks[ ph->left_fork ] );
        ph->total_thinking_time += think( ph->think_time, ph->thread_id );
    }
    printf( "Philosopher %ld total eating time: %ld total thinking time: %ld "
            "starvation: %ld\n", ph->thread_id, ph->total_eating_time,
            ph->total_thinking_time, ph->total_starvation_time );
    return NULL;
}

int
main( int argc, const char* argv[] )
{
    int       i;
    pthread_t t[ NUM_PHILOSOPHERS ];

    for ( i = 0; i < NUM_PHILOSOPHERS; i++ )
    {
        pthread_mutex_init( &forks[ i ], NULL );
    }

    for ( i = 0; i < NUM_PHILOSOPHERS; i++ )
    {
        ph[ i ].think_time = 0.0;
        ph[ i ].eat_time   = 0.0;
        ph[ i ].left_fork  = i;
        ph[ i ].right_fork = i + 1;
        ph[ i ].thread_id  = i;
#ifdef EATPOLITE
        ph[ i ].left_neighb  = i - 1;
        ph[ i ].right_neighb = i + 1;
#endif
        ph[ i ].total_thinking_time   = 0;
        ph[ i ].total_eating_time     = 0;
        ph[ i ].total_starvation_time = 0;
        ph[ i ].spagetti              = SPAGETTI;
    }

    // correct first and last values
    ph[ NUM_PHILOSOPHERS - 1 ].right_fork = 0;
#ifdef EATPOLITE
    ph[ 0 ].left_neighb                     = NUM_PHILOSOPHERS - 1;
    ph[ NUM_PHILOSOPHERS - 1 ].right_neighb = 0;
#endif

    for ( i = 0; i < NUM_PHILOSOPHERS; i++ )
    {
        pthread_create( &t[ i ], NULL, dining, ( void* )&ph[ i ] );
    }

    for ( i = 0; i < NUM_PHILOSOPHERS; i++ )
    {
        pthread_join( t[ i ], NULL );
    }

    return 0;
}
