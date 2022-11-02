/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2019,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

#ifndef SCOREP_TIMER_TICKS_H
#define SCOREP_TIMER_TICKS_H

/**
 * @file
 * Provides the inline function SCOREP_Timer_GetClockTicks() used to obtain
 * timestamps in Score-P. The non-inline timing API is found in
 * SCOREP_Timer_Utils.h. See SCOREP_Timer_Avail.h for the enum of available
 * timers.
 */


/* *INDENT-OFF* */
/* *INDENT-ON* */

#include <UTILS_Error.h>
#include "SCOREP_Timer_Avail.h"
#include <SCOREP_Location.h>

#include <SCOREP_Metric_Management.h>

/* Includes needed by specific timers. */

#if HAVE( BACKEND_SCOREP_TIMER_BGL )
# include <bglpersonality.h>
# include <rts.h>
#endif /* BACKEND_SCOREP_TIMER_BGL */

#if HAVE( BACKEND_SCOREP_TIMER_BGP )
# include <common/bgp_personality.h>
# include <common/bgp_personality_inlines.h>
# include <spi/kernel_interface.h>
#endif /* BACKEND_SCOREP_TIMER_BGP */

#if HAVE( BACKEND_SCOREP_TIMER_BGQ )
# include <firmware/include/personality.h>
# ifdef __GNUC__
#  include <ppu_intrinsics.h>
# endif
# ifndef DEFAULT_FREQ_MHZ
#  error "DEFAULT_FREQ_MHZ not defined. Check your includes."
# endif
#endif /* BACKEND_SCOREP_TIMER_BGQ */

#if HAVE( BACKEND_SCOREP_TIMER_MINGW )
# include <windows.h>
#endif /* BACKEND_SCOREP_TIMER_MINGW */

#if HAVE( BACKEND_SCOREP_TIMER_AIX )
# include <sys/time.h>
# include <sys/systemcfg.h>
#endif /* BACKEND_SCOREP_TIMER_AIX */

#if HAVE( BACKEND_SCOREP_TIMER_MAC )
#include <inttypes.h>
#include <mach/mach_time.h>
#endif /* BACKEND_SCOREP_TIMER_MAC */

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
# include "../timer/scorep_timer_tsc.h"
#endif /* BACKEND_SCOREP_TIMER_TSC */

#if HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY )
# include <sys/time.h>
#endif /* BACKEND_SCOREP_TIMER_GETTIMEOFDAY */

#if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
# include <time.h>
#endif /* BACKEND_SCOREP_TIMER_CLOCK_GETTIME */

/* ************************************** typedefs */

typedef struct scorep_location_timers_data
{
    uint64_t logical_timer_val; /* TODO: ndao: rename to something relevant (used with logical, hwcntr, basicBlock) */
    /* we need a new timer because the instrumentation for the 2 timers is done simult. in LLVM (and compiling the plugin */
    /* is done at scorep compile time */
    //uint64_t logical_stmt_cnt_timer_val; /* same here, used for statements count only */
} scorep_location_timers_data;

/* ************************************** static functions */

/**
 * Platform independent timing functions to be used for providing timestamps
 * to @ref SCOREP_Events and others. Together with SCOREP_Timer_GetClockResolution()
 * one can convert ticks to seconds.
 *
 * @return Wall clock time in ticks.
 */
static inline uint64_t
SCOREP_Timer_GetClockTicks( void )
{
    /* TODO: 1. Convert from static inline to macro to enforce inlining. PGI
     * needs to be extracted into additional function call.
     * 2. Use computed goto instead of switch if available. */
    extern timer_type scorep_timer;

    switch ( scorep_timer )
    {
#if HAVE( BACKEND_SCOREP_TIMER_BGL )
        case TIMER_BGL:
            return ( uint64_t )rts_get_timebase();
#endif  /* BACKEND_SCOREP_TIMER_BGL */

#if HAVE( BACKEND_SCOREP_TIMER_BGP )
        case TIMER_BGP:
            return ( uint64_t )_bgp_GetTimeBase();
#endif  /* BACKEND_SCOREP_TIMER_BGP */

#if HAVE( BACKEND_SCOREP_TIMER_BGQ )
        case TIMER_BGQ:
            return ( uint64_t )__mftb();
#endif  /* BACKEND_SCOREP_TIMER_BGQ */

#if HAVE( BACKEND_SCOREP_TIMER_MINGW )
        case TIMER_MINGW:
        {
            LARGE_INTEGER ticks;
            QueryPerformanceCounter( &ticks );
            return ( uint64_t )ticks.QuadPart;
        }
#endif  /* BACKEND_SCOREP_TIMER_MINGW */

#if HAVE( BACKEND_SCOREP_TIMER_AIX )
        case TIMER_AIX:
        {
            timebasestruct_t tmp;
            read_real_time( &tmp, TIMEBASE_SZ );
            /* Convert to seconds and nanoseconds, if necessary. */
            time_base_to_time( &tmp, TIMEBASE_SZ );
            return ( uint64_t )tmp.tb_high * UINT64_C( 1e9 ) + ( uint64_t )tmp.tb_low;
        }
#endif  /* BACKEND_SCOREP_TIMER_AIX */

#if HAVE( BACKEND_SCOREP_TIMER_MAC )
        case TIMER_MAC:
        {
            extern double scorep_ticks_to_nsec_mac;
            return ( uint64_t )( scorep_ticks_to_nsec_mac * mach_absolute_time() );
        }
#endif  /* BACKEND_SCOREP_TIMER_MAC */

#if HAVE( BACKEND_SCOREP_TIMER_TSC )
        case TIMER_TSC:
        {
            #include "../timer/scorep_timer_tsc.inc.c"
        }
#endif  /* BACKEND_SCOREP_TIMER_TSC */

#if HAVE( BACKEND_SCOREP_TIMER_GETTIMEOFDAY )
        case TIMER_GETTIMEOFDAY:
        {
            struct timeval tp;
            gettimeofday( &tp, 0 );
            return ( uint64_t )tp.tv_sec * UINT64_C( 1000000 ) + ( uint64_t )tp.tv_usec;
        }
#endif  /* BACKEND_SCOREP_TIMER_GETTIMEOFDAY */

#if HAVE( BACKEND_SCOREP_TIMER_CLOCK_GETTIME )
        case TIMER_CLOCK_GETTIME:
        {
            struct timespec time;
            int             result = clock_gettime( SCOREP_TIMER_CLOCK_GETTIME_CLK_ID, &time );
            UTILS_ASSERT( result == 0 );
            return ( uint64_t )time.tv_sec * UINT64_C( 1000000000 ) + ( uint64_t )time.tv_nsec;
        }
#endif  /* BACKEND_SCOREP_TIMER_CLOCK_GETTIME */

        case TIMER_LOGICAL:
        {
            extern size_t timer_subsystem_id;
            extern bool   SCOREP_Timer_Subsystem_Initialized;

            /* timer subsystem registerd and location initialized */
            if ( SCOREP_Timer_Subsystem_Initialized )
            {
                SCOREP_Location*             location       = SCOREP_Location_GetCurrentCPULocation();
                scorep_location_timers_data* subsystem_data =
                    SCOREP_Location_GetSubsystemData( location, timer_subsystem_id );

                /* as well as returning the timer value, getClockTicks function
                   also is called to retrieve each event's timestamp, and
                   therefore logical events counter update will be executed
                   inside this function calls */

                subsystem_data->logical_timer_val++;

                SCOREP_Location_SetSubsystemData( location,
                                                  timer_subsystem_id,
                                                  subsystem_data );

                return subsystem_data->logical_timer_val;
            }
            else
            {
                return 0;
            }
        }

        case TIMER_LOGICAL_HWCTR_INSTR:
        {
            extern size_t timer_subsystem_id;
            extern bool   SCOREP_Timer_Subsystem_Initialized;
            extern bool   SCOREP_Timer_Subsystem_Logic_Event_Sync;

            if ( SCOREP_Timer_Subsystem_Initialized )
            {
                SCOREP_Location*    location      = SCOREP_Location_GetCurrentCPULocation();
                uint64_t*           metric_values = SCOREP_Metric_Read( location );
                scorep_location_timers_data* subsystem_data =
                   SCOREP_Location_GetSubsystemData( location, timer_subsystem_id );

                if (metric_values != NULL)
                {
                    /* must compare with existing value because in children threads
                       number of instructions is less than number of instructions
                       calculated at sync points (master thread has more instr)
                       and so without this check, children threads won't be sync
                       with master threads at all */

                    if (SCOREP_Timer_Subsystem_Logic_Event_Sync == false)
                    {
                        subsystem_data->logical_timer_val = metric_values[0] > subsystem_data->logical_timer_val ?
                                                            metric_values[0] : subsystem_data->logical_timer_val;
                    }
                    /* lamport logical counting mandates that after sync,
                       increment by one */

                    subsystem_data->logical_timer_val++;

                    SCOREP_Location_SetSubsystemData( location,
                                                      timer_subsystem_id,
                                                      subsystem_data );
                }

                return subsystem_data->logical_timer_val;
            }
            else
            {
                return 0;
            }
        }

        case TIMER_LOGICAL_BASIC_BLOCK:
        {
            extern size_t timer_subsystem_id;
            extern bool   SCOREP_Timer_Subsystem_Initialized;

            /* timer subsystem registerd and location initialized */
            if ( SCOREP_Timer_Subsystem_Initialized )
            {
                SCOREP_Location*             location       = SCOREP_Location_GetCurrentCPULocation();
                scorep_location_timers_data* subsystem_data =
                    SCOREP_Location_GetSubsystemData( location, timer_subsystem_id );

                /* TODO: ndao: should we increment counter here by one also ? */
                return subsystem_data->logical_timer_val++;
            }
            else
            {
                return 0;
            }
        }

        // this case might be not needed and the ORING condition of cases with BB case
        case TIMER_LOGICAL_STATEMENT:
        {
            extern size_t timer_subsystem_id;
            extern bool   SCOREP_Timer_Subsystem_Initialized;

            /* timer subsystem registerd and location initialized */
            if ( SCOREP_Timer_Subsystem_Initialized )
            {
                SCOREP_Location*             location       = SCOREP_Location_GetCurrentCPULocation();
                scorep_location_timers_data* subsystem_data =
                    SCOREP_Location_GetSubsystemData( location, timer_subsystem_id );

                /* TODO: ndao: should we increment counter here by one also ? */
                //return subsystem_data->logical_stmt_cnt_timer_val++;
                return subsystem_data->logical_timer_val++;
            }
            else
            {
                return 0;
            }
        }

        default:
        {
            UTILS_FATAL( "Invalid timer selected, shouldn't happen." );
        }
    }
    return 0; /* never reached, but silences warnings */
}


#endif /* SCOREP_TIMER_TICKS_H */
