/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file
 */


#include <config.h>

#include <SCOREP_Timer_Ticks.h>
#include <SCOREP_Timer_Utils.h>

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
#include <scorep_ipc.h>
#include <UTILS_IO.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#endif  /* BACKEND_SCOREP_TIMER_TSC */

/* *INDENT-OFF* */
/* *INDENT-ON*  */


timer_type scorep_timer;  /**< Timer used throughout measurement. Can take
                           * one value out of the enum timer_type. Can be set
                           * before measurement using the SCOREP_TIMER
                           * environment variable. It defaults to the first
                           * available timer. Available timers are detected
                           * at configuration time. */

static bool is_initialized = false;

#if HAVE( BACKEND_SCOREP_TIMER_BGL )
static uint64_t ticks_per_sec_bgl;
#endif /* BACKEND_SCOREP_TIMER_BGL */

#if HAVE( BACKEND_SCOREP_TIMER_BGP )
static uint64_t ticks_per_sec_bgp;
#endif /* BACKEND_SCOREP_TIMER_BGP */

#if HAVE( BACKEND_SCOREP_TIMER_MINGW )
static uint64_t ticks_per_sec_mingw;
#endif /* BACKEND_SCOREP_TIMER_MINGW */

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
static uint64_t timer_tsc_t0;
static uint64_t timer_cmp_t0;
static uint64_t timer_cmp_freq;
#endif  /* BACKEND_SCOREP_TIMER_TSC */


#include "scorep_timer_confvars.inc.c"

void
SCOREP_Timer_Register( void )
{
    SCOREP_ConfigRegister( "", scorep_timer_confvars );
}


void
SCOREP_Timer_Initialize( void )
{
    if ( is_initialized )
    {
        return;
    }

    switch ( scorep_timer )
    {
#if HAVE( BACKEND_SCOREP_TIMER_BGL )
        case TIMER_BGL:
        {
            BGLPersonality mybgl;
            rts_get_personality( &mybgl, sizeof( BGLPersonality ) );
            ticks_per_sec_bgl = ( uint64_t )BGLPersonality_clockHz( &mybgl );
        }
        break;
#endif  /* BACKEND_SCOREP_TIMER_BGL */

#if HAVE( BACKEND_SCOREP_TIMER_BGP )
        case TIMER_BGP:
        {
            _BGP_Personality_t mybgp;
            Kernel_GetPersonality( &mybgp, sizeof( _BGP_Personality_t ) );
            ticks_per_sec_bgp = ( uint64_t )BGP_Personality_clockMHz( &mybgp ) * UINT64_C( 1000000 );
        }
        break;
#endif  /* BACKEND_SCOREP_TIMER_BGP */

#if HAVE( BACKEND_SCOREP_TIMER_BGQ )
        case TIMER_BGQ:
            break;
#endif  /* BACKEND_SCOREP_TIMER_BGQ */

#if HAVE( BACKEND_SCOREP_TIMER_MINGW )
        case TIMER_MINGW:
        {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency( &frequency );
            ticks_per_sec_mingw = ( uint64_t )frequency.QuadPart;
        }
        break;
#endif  /* BACKEND_SCOREP_TIMER_MINGW */

#if HAVE( BACKEND_SCOREP_TIMER_AIX )
        case TIMER_AIX:
            break;
#endif  /* BACKEND_SCOREP_TIMER_AIX */

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
        case TIMER_TSC:
        {
# if !( defined( __FUJITSU ) ) /* For Fujitsu we 1. know that the tsc is nonstop and constant
                                * and 2. /proc/cpuinfo doesn't contain this data. */
            FILE*            fp;
            char*            line           = NULL;
            size_t           length         = 0;
            char             proc_cpuinfo[] = "/proc/cpuinfo";
            SCOREP_ErrorCode status;

            fp = fopen( proc_cpuinfo, "r" );
            if ( fp == NULL )
            {
                UTILS_ERROR_POSIX( "Cannot check for \'nonstop_tsc\' and \'constant_tsc\' "
                                   "in %s. Switch to a timer different from "
                                   "\'tsc\' if you have issues with timings.", proc_cpuinfo );
            }
            else
            {
                bool constant_and_nonstop_tsc = false;
                while ( ( status = UTILS_IO_GetLine( &line, &length, fp ) ) == SCOREP_SUCCESS )
                {
                    if ( strstr( line, "nonstop_tsc" ) != NULL &&
                         strstr( line, "constant_tsc" ) != NULL )
                    {
                        constant_and_nonstop_tsc = true;
                        break;
                    }
                }
                if ( !constant_and_nonstop_tsc )
                {
                    if ( status != SCOREP_ERROR_END_OF_BUFFER )
                    {
                        UTILS_ERROR( status, "Error reading %s for timer consistency check.", proc_cpuinfo );
                    }
                    UTILS_WARNING( "From %s, could not determine if tsc timer is (\'nonstop_tsc\' "
                                   "&& \'constant_tsc\'). Timings likely to be unreliable. "
                                   "Switch to a timer different from \'tsc\' if you have "
                                   "issues with timings.", proc_cpuinfo );
                }
                free( line );
            }
# endif     /* !(defined(__FUJITSU)) */

            /* TODO: assert that all processes use TIMER_TSC running at the
             * same frequency. For this we need to MPP communicate but MPP might
             * not be initialized here. The frequency is interpolated at the end
             * of the measurement, if we detect different frequencies there, this
             * measurement is useless. Both problems can be solved by introducing
             * a timer record per process/location. */
            timer_tsc_t0 = SCOREP_Timer_GetClockTicks();
            /* Either BACKEND_SCOREP_TIMER_CLOCK_GETTIME or
             * BACKEND_SCOREP_TIMER_GETTIMEOFDAY are available, see check in
             * scorep_timer_tsc.h */
# if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
            struct timespec time;
            int             result = clock_gettime( SCOREP_TIMER_CLOCK_GETTIME_CLK_ID, &time );
            UTILS_ASSERT( result == 0 );
            timer_cmp_t0   = ( uint64_t )time.tv_sec * UINT64_C( 1000000000 ) + ( uint64_t )time.tv_nsec;
            timer_cmp_freq = UINT64_C( 1000000000 );
# else      /* HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY ) */
            struct timeval tp;
            gettimeofday( &tp, 0 );
            timer_cmp_t0   =  ( uint64_t )tp.tv_sec * UINT64_C( 1000000 ) + ( uint64_t )tp.tv_usec;
            timer_cmp_freq = UINT64_C( 1000000 );
# endif
        }
        break;
#endif  /* BACKEND_SCOREP_TIMER_TSC */

#if HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY )
        case TIMER_GETTIMEOFDAY:
            break;
#endif  /* BACKEND_SCOREP_TIMER_GETTIMEOFDAY */

#if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
        case TIMER_CLOCK_GETTIME:
            break;
#endif  /* BACKEND_SCOREP_TIMER_CLOCK_GETTIME */

        default:
            UTILS_FATAL( "Invalid timer selected, shouldn't happen." );
    }

    is_initialized = true;
}


uint64_t
SCOREP_Timer_GetClockResolution( void )
{
    switch ( scorep_timer )
    {
#if HAVE( BACKEND_SCOREP_TIMER_BGL )
        case TIMER_BGL:
            return ticks_per_sec_bgl;
#endif  /* BACKEND_SCOREP_TIMER_BGL */

#if HAVE( BACKEND_SCOREP_TIMER_BGP )
        case TIMER_BGP:
            return ticks_per_sec_bgp;
#endif  /* BACKEND_SCOREP_TIMER_BGP */

#if HAVE( BACKEND_SCOREP_TIMER_BGQ )
        case TIMER_BGQ:
            return DEFAULT_FREQ_MHZ * UINT64_C( 1000000 );
#endif  /* BACKEND_SCOREP_TIMER_BGQ */

#if HAVE( BACKEND_SCOREP_TIMER_MINGW )
        case TIMER_MINGW:
            return ticks_per_sec_mingw;
#endif  /* BACKEND_SCOREP_TIMER_MINGW */

#if HAVE( BACKEND_SCOREP_TIMER_AIX )
        case TIMER_AIX:
            return UINT64_C( 1000000000 );
#endif  /* BACKEND_SCOREP_TIMER_AIX */

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
        case TIMER_TSC:
        {
            uint64_t timer_tsc_t1 = SCOREP_Timer_GetClockTicks();
            /* Either BACKEND_SCOREP_TIMER_CLOCK_GETTIME or
             * BACKEND_SCOREP_TIMER_GETTIMEOFDAY are available, see check in
             * scorep_timer_tsc.h */
# if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
            struct timespec time;
            int             result = clock_gettime( SCOREP_TIMER_CLOCK_GETTIME_CLK_ID, &time );
            UTILS_ASSERT( result == 0 );
            uint64_t timer_cmp_t1 = ( uint64_t )time.tv_sec * UINT64_C( 1000000000 ) + ( uint64_t )time.tv_nsec;
# else      /* HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY ) */
            struct timeval tp;
            gettimeofday( &tp, 0 );
            uint64_t timer_cmp_t1 =  ( uint64_t )tp.tv_sec * UINT64_C( 1000000 ) + ( uint64_t )tp.tv_usec;
# endif
            /* Use interpolation to determine frequency.
             * Interpolate and check consistency only once during finalization. */
            static bool     first_visit = true;
            static uint64_t timer_tsc_freq;
            if ( first_visit )
            {
                first_visit    = false;
                timer_tsc_freq = ( double )( timer_tsc_t1 - timer_tsc_t0 ) / ( timer_cmp_t1 - timer_cmp_t0 ) * timer_cmp_freq;

                /* Assert that all processes use roughly the same frequency.
                 * Problems with non-uniform frequencies to be solved by timer
                 * per process/location record.*/
                if ( SCOREP_Ipc_GetRank() != 0 )
                {
                    SCOREP_Ipc_Gather( &timer_tsc_freq, 0, 1, SCOREP_IPC_UINT64_T, 0 );
                }
                else
                {
                    int      size = SCOREP_Ipc_GetSize();
                    uint64_t frequencies[ size ];
                    SCOREP_Ipc_Gather( &timer_tsc_freq,
                                       frequencies,
                                       1,
                                       SCOREP_IPC_UINT64_T,
                                       0 );

                    double sum            = 0;
                    double sum_of_squares = 0;
                    for ( int i = 0; i < size; ++i )
                    {
                        sum            += frequencies[ i ];
                        sum_of_squares += ( double )frequencies[ i ] * frequencies[ i ];
                    }
                    double avg_frequency = sum / size;
                    double stddev        = sqrt( ( sum_of_squares - ( sum * sum ) / size ) / ( size - 1.0 ) );
                    double percent       = stddev * 100 / avg_frequency;
                    double threshold     = 0.00001; /* 'invented' value derived from a 4 process
                                                       run that took 15 seconds. */
                    if ( percent > threshold )
                    {
                        UTILS_WARNING( "Calculated timer (tsc) frequencies differ between processes "
                                       "by more than %f%% (avg_frequency = %f/s; stddev = %f/s; "
                                       "threshold = %f%%). Consider using a timer with a fixed "
                                       "frequency like gettimeofday or clock_gettime or prolong "
                                       "the measurement duration.",
                                       percent, avg_frequency, stddev, threshold );
                        for ( int i = 0; i < size; ++i )
                        {
                            printf( "rank[%d]:\t frequency = %" PRIu64 "/s \n", i, frequencies[ i ] );
                        }
                    }
                }
            }
            return timer_tsc_freq;
        }
#endif  /* BACKEND_SCOREP_TIMER_TSC */

#if HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY )
        case TIMER_GETTIMEOFDAY:
            return UINT64_C( 1000000 );
#endif  /* BACKEND_SCOREP_TIMER_GETTIMEOFDAY */

#if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
        case TIMER_CLOCK_GETTIME:
            return UINT64_C( 1000000000 );
#endif  /* BACKEND_SCOREP_TIMER_CLOCK_GETTIME */

        default:
            UTILS_FATAL( "Invalid timer selected, shouldn't happen." );
    }
}


bool
SCOREP_Timer_ClockIsGlobal( void )
{
    switch ( scorep_timer )
    {
#if HAVE( BACKEND_SCOREP_TIMER_BGL )
        case TIMER_BGL:
            return true;
#endif  /* BACKEND_SCOREP_TIMER_BGL */

#if HAVE( BACKEND_SCOREP_TIMER_BGP )
        case TIMER_BGP:
            return true;
#endif  /* BACKEND_SCOREP_TIMER_BGP */

#if HAVE( BACKEND_SCOREP_TIMER_BGQ )
        case TIMER_BGQ:
            return true;
#endif  /* BACKEND_SCOREP_TIMER_BGQ */

#if HAVE( BACKEND_SCOREP_TIMER_MINGW )
        case TIMER_MINGW:
            return false;
#endif  /* BACKEND_SCOREP_TIMER_MINGW */

#if HAVE( BACKEND_SCOREP_TIMER_AIX )
        case TIMER_MINGW:
            return false;
#endif  /* BACKEND_SCOREP_TIMER_AIX */

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
        case TIMER_TSC:
            return false;
#endif  /* BACKEND_SCOREP_TIMER_TSC */

#if HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY )
        case TIMER_GETTIMEOFDAY:
            return false;
#endif  /* BACKEND_SCOREP_TIMER_GETTIMEOFDAY */

#if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
        case TIMER_CLOCK_GETTIME:
            return false;
#endif  /* BACKEND_SCOREP_TIMER_CLOCK_GETTIME */

        default:
            UTILS_FATAL( "Invalid timer selected, shouldn't happen." );
    }
}
