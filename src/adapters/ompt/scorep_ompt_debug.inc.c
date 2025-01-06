/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * Collection of 'ompt to string' functions for debugging.
 */


#if HAVE( UTILS_DEBUG )

static const char*
thread2string( ompt_thread_t t )
{
    switch ( t )
    {
        case ompt_thread_initial:
            return "initial";
        case ompt_thread_worker:
            return "worker";
        case ompt_thread_other:
            return "other";
        case ompt_thread_unknown:
            return "unknown";
    }
    UTILS_BUG( "invalid thread type '%d' provided", t );
    return "";
}

static const char*
parallel_flag2string( ompt_parallel_flag_t t )
{
    /*
       The value ompt_parallel_invoker_program indicates that the
       outlined function associated with implicit tasks for the region
       is invoked directly by the application on the primary thread
       for a parallel region. The value ompt_parallel_invoker_runtime
       indicates that the outlined function associated with implicit
       tasks for the region is invoked by the runtime on the primary
       thread for a parallel region.

       The value ompt_parallel_league indicates that the callback is
       invoked due to the creation of a league of teams by a teams
       construct. The value ompt_parallel_team indicates that the
       callback is invoked due to the creation of a team of threads by
       a parallel construct.
     */
    if ( t & ompt_parallel_invoker_program )
    {
        if ( t & ompt_parallel_league )
        {
            UTILS_BUG_ON( t != ( ompt_parallel_invoker_program | ompt_parallel_league ) );
            return "program_league";
        }
        if ( t & ompt_parallel_team )
        {
            UTILS_BUG_ON( t != ( ompt_parallel_invoker_program | ompt_parallel_team ) );
            return "program_team";
        }
        UTILS_BUG();
    }
    if ( t & ompt_parallel_invoker_runtime )
    {
        if ( t & ompt_parallel_league )
        {
            UTILS_BUG_ON( t != ( ompt_parallel_invoker_runtime | ompt_parallel_league ) );
            return "runtime_league";
        }
        if ( t & ompt_parallel_team )
        {
            UTILS_BUG_ON( t != ( ompt_parallel_invoker_runtime | ompt_parallel_team ) );
            return "runtime_team";
        }
        UTILS_BUG();
    }
    UTILS_BUG();
    return "";
}

static const char*
scope_endpoint2string( ompt_scope_endpoint_t t )
{
    switch ( t )
    {
        case ompt_scope_begin:
            return "begin";
        case ompt_scope_end:
            return "end";
        #if HAVE( DECL_OMPT_SCOPE_BEGINEND )
        case ompt_scope_beginend:
            return "beginend";
        #endif
    }
    UTILS_BUG();
    return "";
}

static const char*
task_flag2string( ompt_task_flag_t t )
{
    /*
       ompt_task_initial = 0x00000001,
       ompt_task_implicit = 0x00000002,
       ompt_task_explicit = 0x00000004,
       ompt_task_target = 0x00000008,
       ompt_task_taskwait = 0x00000010,
       ompt_task_undeferred = 0x08000000,
       ompt_task_untied = 0x10000000,
       ompt_task_final = 0x20000000,
       ompt_task_mergeable = 0x40000000,
       ompt_task_merged = 0x80000000
     */

    /* The flags argument indicates the kind of task (explicit or
       target) that is generated. Values for flags are a disjunction
       of elements in the ompt_task_flag_t enumeration type.

       The ompt_task_flag_t enumeration type defines valid task type
       values. The least significant byte provides information about
       the general classification of the task. The other bits
       represent properties of the task.
     */
    if ( t & ompt_task_initial )
    {
        UTILS_BUG_ON( t != ompt_task_initial );
        return "initial";
    }
    if ( t & ompt_task_implicit )
    {
        if ( t & ompt_task_undeferred )
        {
            return "implicit_undeferred";
        }
        if ( t & ompt_task_untied )
        {
            return "implicit_untied";
        }
        if ( t & ompt_task_final )
        {
            return "implicit_final";
        }
        if ( t & ompt_task_mergeable )
        {
            return "implicit_mergeable";
        }
        if ( t & ompt_task_merged )
        {
            return "implicit_merged";
        }
        UTILS_BUG_ON( t != ompt_task_implicit );
        return "implicit";
    }
    if ( t & ompt_task_explicit )
    {
        if ( t & ompt_task_undeferred )
        {
            return "explicit_undeferred";
        }
        if ( t & ompt_task_untied )
        {
            return "explicit_untied";
        }
        if ( t & ompt_task_final )
        {
            return "explicit_final";
        }
        if ( t & ompt_task_mergeable )
        {
            return "explicit_mergeable";
        }
        if ( t & ompt_task_merged )
        {
            return "explicit_merged";
        }
        UTILS_BUG_ON( t != ompt_task_explicit );
        return "explicit";
    }
    if ( t & ompt_task_target )
    {
        if ( t & ompt_task_undeferred )
        {
            return "target_undeferred";
        }
        if ( t & ompt_task_untied )
        {
            return "target_untied";
        }
        if ( t & ompt_task_final )
        {
            return "target_final";
        }
        if ( t & ompt_task_mergeable )
        {
            return "target_mergeable";
        }
        if ( t & ompt_task_merged )
        {
            return "target_merged";
        }
        UTILS_BUG_ON( t != ompt_task_target );
        return "target";
    }
    #if HAVE( DECL_OMPT_TASK_TASKWAIT )
    if ( t & ompt_task_taskwait )
    {
        if ( t & ompt_task_undeferred )
        {
            return "taskwait_undeferred";
        }
        if ( t & ompt_task_untied )
        {
            return "taskwait_untied";
        }
        if ( t & ompt_task_final )
        {
            return "taskwait_final";
        }
        if ( t & ompt_task_mergeable )
        {
            return "taskwait_mergeable";
        }
        if ( t & ompt_task_merged )
        {
            return "taskwait_merged";
        }
        UTILS_BUG_ON( t != ompt_task_taskwait );
        return "taskwait";
    }
    #endif /* DECL_OMPT_TASK_TASKWAIT */
    UTILS_BUG();
    return "";
}

#endif /* HAVE( UTILS_DEBUG ) */

static const char*
task_status2string( ompt_task_status_t t )
{
    switch ( t )
    {
        case ompt_task_complete:
            return "complete";
        case ompt_task_yield:
            return "yield";
        case ompt_task_cancel:
            return "cancel";
        case ompt_task_detach:
            return "detach";
        case ompt_task_early_fulfill:
            return "early_fulfill";
        case ompt_task_late_fulfill:
            return "late_fulfill";
        case ompt_task_switch:
            return "switch";
        case ompt_taskwait_complete:
            return "taskwait_complete";
    }
    UTILS_BUG();
    return "";
}

#if HAVE( SCOREP_OMPT_TARGET_SUPPORT )

static const char*
target_data_optype2string( ompt_target_data_op_t t )
{
    /**
     * typedef enum ompt_target_data_op_t {
          ompt_target_data_alloc                      = 1,
          ompt_target_data_transfer_to_device         = 2,
          ompt_target_data_transfer_from_device       = 3,
          ompt_target_data_delete                     = 4,
          ompt_target_data_associate                  = 5,
          ompt_target_data_disassociate               = 6,
          ompt_target_data_alloc_async                = 17,
          ompt_target_data_transfer_to_device_async   = 18,
          ompt_target_data_transfer_from_device_async = 19,
          ompt_target_data_delete_async               = 20
        } ompt_target_data_op_t;
     */
    switch ( t )
    {
        case ompt_target_data_alloc:
            return "target_data_alloc";
        case ompt_target_data_transfer_to_device:
            return "target_data_transfer_to_device";
        case ompt_target_data_transfer_from_device:
            return "target_data_transfer_from_device";
        case ompt_target_data_delete:
            return "target_data_delete";
        #if HAVE( DECL_OMPT_TARGET_DATA_ASSOCIATE )
        case ompt_target_data_associate:
            return "target_data_associate";
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_ASSOCIATE ) */
        #if HAVE( DECL_OMPT_TARGET_DATA_DISASSOCIATE )
        case ompt_target_data_disassociate:
            return "target_data_disassociate";
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_DISASSOCIATE ) */
        #if HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC )
        case ompt_target_data_alloc_async:
            return "target_data_alloc_async";
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC ) */
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC )
        case ompt_target_data_transfer_to_device_async:
            return "target_data_transfer_to_device_async";
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC ) */
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC )
        case ompt_target_data_transfer_from_device_async:
            return "target_data_transfer_from_device_async";
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC ) */
        #if HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC )
        case ompt_target_data_delete_async:
            return "target_data_delete_async";
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC ) */
        default:
            break;
    }
    UTILS_BUG();
    return "";
}

static const char*
target_kind2string( ompt_target_t t )
{
    /**
     * typedef enum ompt_target_t {
          ompt_target                         = 1,
          ompt_target_enter_data              = 2,
          ompt_target_exit_data               = 3,
          ompt_target_update                  = 4,
          ompt_target_nowait                  = 9,
          ompt_target_enter_data_nowait       = 10,
          ompt_target_exit_data_nowait        = 11,
          ompt_target_update_nowait           = 12
        } ompt_target_t;
     */
    switch ( t )
    {
        case ompt_target:
            return "target";
        case ompt_target_enter_data:
            return "target_enter_data";
        case ompt_target_exit_data:
            return "target_exit_data";
        case ompt_target_update:
            return "target_update";
        #if HAVE( DECL_OMPT_TARGET_NOWAIT )
        case ompt_target_nowait:
            return "target_nowait";
        #endif /* HAVE( DECL_OMPT_TARGET_NOWAIT ) */
        #if HAVE( DECL_OMPT_TARGET_ENTER_DATA_NOWAIT )
        case ompt_target_enter_data_nowait:
            return "target_enter_data_nowait";
        #endif /* HAVE( DECL_OMPT_TARGET_ENTER_DATA_NOWAIT ) */
        #if HAVE( DECL_OMPT_TARGET_EXIT_DATA_NOWAIT )
        case ompt_target_exit_data_nowait:
            return "target_exit_data_nowait";
        #endif /* HAVE( DECL_OMPT_TARGET_EXIT_DATA_NOWAIT ) */
        #if HAVE( DECL_OMPT_TARGET_UPDATE_NOWAIT )
        case ompt_target_update_nowait:
            return "target_update_nowait";
        #endif /* HAVE( DECL_OMPT_TARGET_UPDATE_NOWAIT ) */
        default:
            break;
    }
    UTILS_BUG();
    return "";
}

#endif /* HAVE( SCOREP_OMPT_TARGET_SUPPORT ) */

static const char*
sync_region2string( ompt_sync_region_t t )
{
    switch ( t )
    {
        case ompt_sync_region_barrier:
            return "barrier (deprecated)";
        case ompt_sync_region_barrier_implicit:
            return "barrier_implicit (deprecated)";
        case ompt_sync_region_barrier_explicit:
            return "barrier_explicit";
        case ompt_sync_region_barrier_implementation:
            return "barrier_implementation";
        case ompt_sync_region_taskwait:
            return "taskwait";
        case ompt_sync_region_taskgroup:
            return "taskgroup";
        case ompt_sync_region_reduction:
            return "reduction";
        #if HAVE( DECL_OMPT_SYNC_REGION_BARRIER_IMPLICIT_WORKSHARE )
        case ompt_sync_region_barrier_implicit_workshare:
            return "barrier_implicit_workshare";
        #endif
        #if HAVE( DECL_OMPT_SYNC_REGION_BARRIER_IMPLICIT_PARALLEL )
        case ompt_sync_region_barrier_implicit_parallel:
            return "barrier_implicit_parallel";
        #endif
        #if HAVE( DECL_OMPT_SYNC_REGION_BARRIER_TEAMS )
        case ompt_sync_region_barrier_teams:
            return "barrier_teams";
        #endif
    }
    UTILS_BUG();
    return "";
}

static const char*
work2string( ompt_work_t t )
{
    switch ( t )
    {
        case ompt_work_loop:
            return "loop";
        case ompt_work_sections:
            return "sections";
        case ompt_work_single_executor:
            return "single_executor";
        case ompt_work_single_other:
            return "single_other";
        case ompt_work_workshare:
            return "workshare";
        case ompt_work_distribute:
            return "distribute";
        case ompt_work_taskloop:
            return "taskloop";
        case ompt_work_scope:
            return "scope";
        #if HAVE( DECL_OMPT_WORK_LOOP_STATIC )
        case ompt_work_loop_static:
            return "loop_static";
        #endif
        #if HAVE( DECL_OMPT_WORK_LOOP_DYNAMIC )
        case ompt_work_loop_dynamic:
            return "loop_dynamic";
        #endif
        #if HAVE( DECL_OMPT_WORK_LOOP_GUIDED )
        case ompt_work_loop_guided:
            return "loop_guided";
        #endif
        #if HAVE( DECL_OMPT_WORK_LOOP_OTHER )
        case ompt_work_loop_other:
            return "loop_other";
        #endif
    }
    UTILS_BUG();
    return "";
}

static const char*
looptype2string( ompt_work_t t )
{
    switch ( t )
    {
        case ompt_work_loop:
            return "unknown";
#if HAVE( DECL_OMPT_WORK_LOOP_STATIC )
        case ompt_work_loop_static:
            return "static";
#endif
#if HAVE( DECL_OMPT_WORK_LOOP_DYNAMIC )
        case ompt_work_loop_dynamic:
            return "dynamic";
#endif
#if HAVE( DECL_OMPT_WORK_LOOP_GUIDED )
        case ompt_work_loop_guided:
            return "guided";
#endif
#if HAVE( DECL_OMPT_WORK_LOOP_OTHER )
        case ompt_work_loop_other:
            return "implementation specific";
#endif
        default:
            UTILS_BUG();
            return "";
    }
}

static const char*
mutex2string( ompt_mutex_t t )
{
    switch ( t )
    {
        case ompt_mutex_lock:
            return "lock";
        case ompt_mutex_test_lock:
            return "test_lock";
        case ompt_mutex_nest_lock:
            return "nest_lock";
        case ompt_mutex_test_nest_lock:
            return "test_nest_lock";
        case ompt_mutex_critical:
            return "critical";
        case ompt_mutex_atomic:
            return "atomic";
        case ompt_mutex_ordered:
            return "ordered";
    }
    UTILS_BUG();
    return "";
}

static const char*
dispatch2string( ompt_dispatch_t t )
{
    switch ( t )
    {
        case ompt_dispatch_iteration:
            return "iteration";
        case ompt_dispatch_section:
            return "section";
        #if HAVE( DECL_OMPT_DISPATCH_WS_LOOP_CHUNK )
        case ompt_dispatch_ws_loop_chunk:
            return "ws_loop_chunk";
        #endif
        #if HAVE( DECL_OMPT_DISPATCH_TASKLOOP_CHUNK )
        case ompt_dispatch_taskloop_chunk:
            return "taskloop_chunk";
        #endif
        #if HAVE( DECL_OMPT_DISPATCH_DISTRIBUTE_CHUNK )
        case ompt_dispatch_distribute_chunk:
            return "distribute_chunk";
        #endif
    }
    UTILS_BUG();
    return "";
}
