import sys
import otf2
from otf2.events import *

threadForkEvents_cnt                = 0
threadJoinEvents_cnt                = 0
threadForkEvents_timestamps         = []
threadJoinEvents_timestamps         = []
threadAcquireLockEvents             = [[]]
threadReleaseLockEvents             = [[]]
ompEnterEvents                      = [[]]
ompLeaveEvents                      = [[]]
ompBarrierEnterEvents_timestamps    = []
ompBarrierLeaveEvents_timestamps    = []
ompImpBarrierEnterEvents_timestamps = []
ompImpBarrierLeaveEvents_timestamps = []
ompCriticalEnterEvents_timestamps   = []
ompCriticalLeaveEvents_timestamps   = []

# Trace validation functions for OpenMP events

# function validates that timestamp for ThreadFork event
def val_threadFork( num_threads, threadForkEvents_cnt,  threadForkEvents_timestamps ):
    arrEntriesForForkEvent = num_threads + 1         # master has 2 entries, as fork and as teamBegin
    for j in range ( threadForkEvents_cnt ):
        forkEvent_idx           = arrEntriesForForkEvent * j
        teamBeginEventStart_idx = 1 + ( arrEntriesForForkEvent * j )
        teamBeginEventStop_idx  = arrEntriesForForkEvent * ( j + 1 )
        for i in range ( teamBeginEventStart_idx, teamBeginEventStop_idx ):
            if ( threadForkEvents_timestamps[ i ] != ( threadForkEvents_timestamps[ forkEvent_idx ] + 1 ) ):
                return False
    return True

# function validates that timestamp for ThreadJoin event
def val_threadJoin( num_threads, threadJoinEvents_cnt,  threadJoinEvents_timestamps ):
    arrEntriesForJoinEvent = num_threads + 1        # master has 2 entries, as teamEnd and as join
    for j in range ( threadJoinEvents_cnt ):
        joinEvent_idx         = num_threads + ( arrEntriesForJoinEvent * j )
        teamEndEventStart_idx = arrEntriesForJoinEvent * j
        teamEndEventStop_idx  = 1 + ( arrEntriesForJoinEvent * j )
        for i in range ( teamEndEventStart_idx, teamEndEventStop_idx ):
            if ( threadJoinEvents_timestamps[ i ] + 1 != ( threadJoinEvents_timestamps[ joinEvent_idx ] ) ):
                return False
    return True

def val_barrier( num_threads, ompEnterBarrierEvents_timestamps, ompLeaveBarrierEvents_timestamps):
    num_barrierEvents = int( len( ompEnterBarrierEvents_timestamps ) / num_threads )
    for i in range ( num_barrierEvents ):
        start         = ( num_threads + 1 ) * i
        end           = start + num_threads
        # get max for first Barrier entry
        max_timestmap = max ( ompEnterBarrierEvents_timestamps[ start : end ] )
        # check leave timestamps for threads
        for leaveTimestamp in ompLeaveBarrierEvents_timestamps[ start : end ]:
            if ( leaveTimestamp != max_timestmap + 1 ):
                return False
    return True

# critical acquires lock -> val_lock test is sufficient
def val_critical( num_threads, ompCriticalEnterEvents_timestamps, ompCriticalLeaveEvents_timestamps ):
    pass

def val_lock( num_threads, threadAcquireLockEvents, threadReleaseLockEvents ):
    del threadAcquireLockEvents[0]
    del threadReleaseLockEvents[0]
    ACQ_ORDER_IDX = 0
    LOCK_IDX      = 1
    TIMESTAMP_IDX = 3
    num_locks     = int( len( threadAcquireLockEvents ) / num_threads )
    # sort with acquisition order for each lock
    threadAcquireLockEvents = sorted( threadAcquireLockEvents, key = lambda x: ( x[ LOCK_IDX ] ) )
    threadReleaseLockEvents = sorted( threadReleaseLockEvents, key = lambda x: ( x[ LOCK_IDX ] ) )
    for i in range ( num_locks ):
        start = ( num_threads ) * i
        end   = start + num_threads - 1
        for j in range ( start, end ):
            if ( threadReleaseLockEvents[ j ][ TIMESTAMP_IDX ] + 1 != threadAcquireLockEvents[ j+1 ][ TIMESTAMP_IDX ] ):
                return False
    return True

#------------------------------------------------------------------------------------------------------------------------#

def read_trace( trace_name ):
    global threadForkEvents_cnt
    global threadJoinEvents_cnt
    with otf2.reader.open(trace_name) as trace:
        for location, event in trace.events:
            if isinstance(event, ThreadFork):
                threadForkEvents_cnt += 1
                threadForkEvents_timestamps.append(event.time)
            elif isinstance(event, ThreadTeamBegin):
                threadForkEvents_timestamps.append(event.time)
            elif isinstance(event, ThreadJoin):
                threadJoinEvents_cnt += 1
                threadJoinEvents_timestamps.append(event.time)
            elif isinstance(event, ThreadTeamEnd):
                threadJoinEvents_timestamps.append(event.time)
            elif isinstance(event, ThreadAcquireLock):
                threadAcquireLockEvents.append([event.acquisition_order, event.lock_id, location.name, event.time])
            elif isinstance(event, ThreadReleaseLock):
                threadReleaseLockEvents.append([event.acquisition_order, event.lock_id, location.name, event.time])
            elif isinstance(event, Enter):
                ompEnterEvents.append([event.region.region_role, location.name, event.time])
            elif isinstance(event, Leave):
                ompLeaveEvents.append([event.region.region_role, location.name, event.time])
            else:
                pass

# Function needed to filter events under testing from Enter & Leave events
# var :  omp_Events: array with events
#        type values: Enter, Leave
def filter_enter_leave_events( omp_Events, type ):
    del omp_Events[0]
    REGION_ROLE_IDX = 0
    TIME_IDX        = 2
    for event in omp_Events:
        if ( str(event[REGION_ROLE_IDX]) == "RegionRole.BARRIER" ):
            if ( type == 'Enter' ):
                ompBarrierEnterEvents_timestamps.append(event[TIME_IDX])
            elif ( type == 'Leave' ):
                ompBarrierLeaveEvents_timestamps.append(event[TIME_IDX])
        elif ( str(event[REGION_ROLE_IDX]) == "RegionRole.IMPLICIT_BARRIER" ):
            if ( type == 'Enter' ):
                ompImpBarrierEnterEvents_timestamps.append(event[TIME_IDX])
            elif ( type == 'Leave' ):
                ompImpBarrierLeaveEvents_timestamps.append(event[TIME_IDX])
        elif ( str(event[REGION_ROLE_IDX]) == "RegionRole.CRITICAL" ):
            if ( type == 'Enter' ):
                ompCriticalEnterEvents_timestamps.append(event[TIME_IDX])
            elif ( type == 'Leave' ):
                ompCriticalLeaveEvents_timestamps.append(event[TIME_IDX])
        else:
            pass

def validate_trace( num_threads, num_processes, test_name ):
    if ( test_name == 'omp_fork_join' ):
        return (    val_threadFork( num_threads, threadForkEvents_cnt, threadForkEvents_timestamps )
                and val_threadJoin( num_threads, threadJoinEvents_cnt, threadJoinEvents_timestamps ) )
    elif ( test_name == 'omp_barrier' ):
        filter_enter_leave_events( ompEnterEvents, 'Enter' )
        filter_enter_leave_events( ompLeaveEvents, 'Leave' )
        return (    val_barrier( num_threads, ompBarrierEnterEvents_timestamps, ompBarrierLeaveEvents_timestamps )
                and val_barrier( num_threads, ompImpBarrierEnterEvents_timestamps, ompImpBarrierLeaveEvents_timestamps ) )
    #elif ( test_name == 'val_critical' ):
    #    filter_enter_leave_events( ompEnterEvents, 'Enter' )
    #    filter_enter_leave_events( ompLeaveEvents, 'Leave' )
    #    return ( val_critical( int( num_threads ), ompCriticalEnterEvents_timestamps, ompCriticalLeaveEvents_timestamps ) )
    # Validation does not work if there is more than one MPI rank
    #elif ( test_name == 'hybrid_reduction' ):
        #return (    val_threadFork( num_threads, threadForkEvents_cnt, threadForkEvents_timestamps ) \
                #and val_threadJoin( num_threads, threadJoinEvents_cnt, threadJoinEvents_timestamps ) )
    elif ( test_name ==  'omp_lock' ):
        return ( val_lock( num_threads, threadAcquireLockEvents, threadReleaseLockEvents ) )
    else:
        return True

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description = 'This script validates logical timer behavior for OMP events')
    parser.add_argument("-i","--input", help="Trace file with path", required=True)
    parser.add_argument("-nt", "--num_threads", type=int, help="Number of threads for OMP program", required=True)
    parser.add_argument("-np", "--num_processes", type=int, help="Number of threads for MPI program", required=True)
    parser.add_argument("-test", "--test_name", help="Name of test", required=True)
    args = parser.parse_args()
    read_trace( args.input )
    is_valid = validate_trace( args.num_threads, args.num_processes, args.test_name )

    # translate to shell return codes
    if is_valid:
        sys.exit(0)
    else:
        sys.exit(1)
