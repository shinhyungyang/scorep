/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

#ifndef POMP2_LIB_H
#define POMP2_LIB_H

#include <stdint.h>

/**
 * @file       pomp_lib.h
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 * @status     ALPHA
 * @ingroup    POMP2
 *
 * @brief Declares public visible items of the POMP2 adapter.
 */

#include <stddef.h>
#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * @defgroup POMP2 POMP2 Adapter implementation
 * This module implements the adapter functions fo the POMP2 adapter. POMP2 defines a set
 * of functions inserted by the source-to-source instrumenter OPARI. OPARI instruments
 * OpenMP construct.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Thread private variable that is passed to child threads via copyin.
 * Can't be easily declared here because of Fortran name mangling issues.
 * Needs to be declared in OPARIs inc files as OPARI can generate all
 * possible mangled names via command line option. */
//extern uint64_t pomp_tpd;
//#ifdef _OPENMP
//#pragma omp threadprivate(pomp_tpd)
//#endif

/** Defines opaque pomp region handle type */
typedef void* POMP2_Region_handle;

/** @name Functions generated by the instrumenter */
/*@{*/
/**
 * Returns the number of instrumented regions.@n
 * The instrumenter scans all opari-created include files with nm and greps
 * the POMP2_INIT_uuid_numRegions() function calls. Here we return the sum of
 * all numRegions.
 */
extern size_t
POMP2_Get_num_regions();

/**
 * Init all opari-created regions.@n
 * The instrumentor scans all opari-created include files with nm and greps
 * the POMP2_INIT_uuid_numRegions() function calls. The instrumentor then
 * defines this functions by calling all grepped functions.
 */
extern void
POMP2_Init_regions();

/*@}*/

/** Initializes the POMP2 adapter. It is inserted at the #pragma pomp inst begin.
 */
extern void
POMP2_Init();

/** Finalizes the POMP2 adapter. It is inserted at the #pragma pomp inst end.
 */
extern void
POMP2_Finalize();

/** Disables the POMP2 adapter.
 */
extern void
POMP2_Off();

/** Enables the POMP2 adapter.
 */
extern void
POMP2_On();

/** Called at the begin of a user defined POMP2 region.
    @param pomp_handle  The handle of the started region.
 */
extern void
POMP2_Begin( POMP2_Region_handle* pomp_handle );

/** Called at the end of a user defined POMP2 region.
    @param pomp_handle  The handle of the ended region.
 */
extern void
POMP2_End( POMP2_Region_handle* pomp_handle );

/** Registers a POMP2 region and returns a region handle.
    @param pomp_handle  Returns the handle for the newly registered region.
    @param ctc_string   A string containing the region data.
 */
extern void
POMP2_Assign_handle( POMP2_Region_handle* pomp_handle,
                     const char           init_string[] );

#ifdef _OPENMP
/** Called before an atomic statement.
    @param pomp_handle  The handle of the started region.
 */
extern void
POMP2_Atomic_enter( POMP2_Region_handle* pomp_handle );

/** Called after an atomic statement.
    @param pomp_handle  The handle of the ended region.
 */
extern void
POMP2_Atomic_exit( POMP2_Region_handle* pomp_handle );

/** Called before an barrier.
    @param pomp_handle  The handle of the started region.
 */
extern void
POMP2_Barrier_enter( POMP2_Region_handle* pomp_handle );

/** Called after an barrier.
    @param pomp_handle  The handle of the ended region.
 */
extern void
POMP2_Barrier_exit( POMP2_Region_handle* pomp_handle );

/** Called before an flush.
    @param pomp_handle  The handle of the started region.
 */
extern void
POMP2_Flush_enter( POMP2_Region_handle* pomp_handle );

/** Called after an flush.
    @param pomp_handle  The handle of the ended region.
 */
extern void
POMP2_Flush_exit( POMP2_Region_handle* pomp_handle );

/** Called at the start of a critical region.
    @param pomp_handle  The handle of the started region.
 */
extern void
POMP2_Critical_begin( POMP2_Region_handle* pomp_handle );

/** Called at the end of a critical region.
    @param pomp_handle  The handle of the ended region.
 */
extern void
POMP2_Critical_end( POMP2_Region_handle* pomp_handle );

/** Called before a critical region.
    @param pomp_handle  The handle of the started region.
 */
extern void
POMP2_Critical_enter( POMP2_Region_handle* pomp_handle );

/** Called after a critical region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Critical_exit( POMP2_Region_handle* pomp_handle );

/** Called before a for loop.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_For_enter( POMP2_Region_handle* pomp_handle );

/** Called after a for loop.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_For_exit( POMP2_Region_handle* pomp_handle );

/** Called at the start of a master region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Master_begin( POMP2_Region_handle* pomp_handle );

/** Called at the end of a master region.
    @param pomp_handle  The handle of the ended region.
 */
extern void
POMP2_Master_end( POMP2_Region_handle* pomp_handle );

/** Called at the start of a parallel region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Parallel_begin( POMP2_Region_handle* pomp_handle );

/** Called at the end of a parallel region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Parallel_end( POMP2_Region_handle* pomp_handle );

/** Called before a parallel region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Parallel_fork( POMP2_Region_handle* pomp_handle,
                     int                  num_threads );

/** Called after a parallel region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Parallel_join( POMP2_Region_handle* pomp_handle );

/** Called at the start of a section.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Section_begin( POMP2_Region_handle* pomp_handle );

/** Called at the end of a section.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Section_end( POMP2_Region_handle* pomp_handle );

/** Called before a set of sections.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Sections_enter( POMP2_Region_handle* pomp_handle );

/** Called after a set of sections.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Sections_exit( POMP2_Region_handle* pomp_handle );

/** Called at the start of a single region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Single_begin( POMP2_Region_handle* pomp_handle );

/** Called at the end of a single region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Single_end( POMP2_Region_handle* pomp_handle );

/** Called before a single region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Single_enter( POMP2_Region_handle* pomp_handle );

/** Called after a single region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Single_exit( POMP2_Region_handle* pomp_handle );

/** Called before a workshare region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Workshare_enter( POMP2_Region_handle* pomp_handle );

/** Called after a workshare region.
    @param pomp_handle  The handle of the region.
 */
extern void
POMP2_Workshare_exit( POMP2_Region_handle* pomp_handle );

/** Wraps the omp_init_lock function */
extern void
POMP2_Init_lock( omp_lock_t* s );

/** Wraps the omp_destroy_lock function */
extern void
POMP2_Destroy_lock( omp_lock_t* s );

/** Wraps the omp_set_lock function */
extern void
POMP2_Set_lock( omp_lock_t* s );

/** Wraps the omp_unset_lock function */
extern void
POMP2_Unset_lock( omp_lock_t* s );

/** Wraps the omp_test_lock function */
extern int
POMP2_Test_lock( omp_lock_t* s );

/** Wraps the omp_init_nest_lock function */
extern void
POMP2_Init_nest_lock( omp_nest_lock_t* s );

/** Wraps the omp_destroy_nest_lock function */
extern void
POMP2_Destroy_nest_lock( omp_nest_lock_t* s );

/** Wraps the omp_set_nest_lock function */
extern void
POMP2_Set_nest_lock( omp_nest_lock_t* s );

/** Wraps the omp_unset_nest_lock function */
extern void
POMP2_Unset_nest_lock( omp_nest_lock_t* s );

/** Wraps the omp_test_nest_lock function */
extern int
POMP2_Test_nest_lock( omp_nest_lock_t* s );

#endif /* _OPENMP */

#ifdef __cplusplus
}
#endif
/** @} */
#endif
