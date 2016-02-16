/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014, 2016,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 * @ingroup    SHMEM_Wrapper
 *
 * @brief Registration of SHMEM functions
 * For all SHMEM functions a region is registered at initialization
 * time of the SHMEM adapter.
 */

#ifndef SCOREP_SHMEMWRAP_REG_H
#define SCOREP_SHMEMWRAP_REG_H

#include <SCOREP_Definitions.h>
#include <SCOREP_Shmem.h>

/*
 * In all modes:
 * - Definition of the CALL_SHMEM macro
 *
 * In static mode:
 * - Declaration of Score-P region handles for wrapped SHMEM functions
 * - Declaration of '__real_' functions
 *
 * In shared mode:
 * - Declaration of Score-P region handles for wrapped SHMEM functions
 * - (@todo) Declaration of pointers to functions in SHMEM lib
 *
 * In weak mode:
 * - Declaration of Score-P region handles for wrapped SHMEM functions
 * - Declaration of strong function symbols
 */

/*
 * STATIC
 */
#ifdef SCOREP_LIBWRAP_STATIC

#define SCOREP_SHMEM_PROCESS_FUNC( type, return_type, func, func_args )   \
    extern SCOREP_RegionHandle scorep_shmem_region__ ## func;       \
    return_type func func_args; \
    return_type      __real_ ## func func_args;

#define CALL_SHMEM( name ) __real_##name


/*
 * SHARED
 */
#elif SCOREP_LIBWRAP_SHARED

#define SCOREP_SHMEM_PROCESS_FUNC( type, return_type, func, func_args )   \
    extern SCOREP_RegionHandle scorep_shmem_region__ ## func;

#error scorep_shmem_func_ptr_##name


/*
 * WEAK
 */
#elif SCOREP_LIBWRAP_WEAK

#define SCOREP_SHMEM_PROCESS_FUNC( type, return_type, func, func_args )   \
    extern SCOREP_RegionHandle scorep_shmem_region__ ## func;       \
    return_type func func_args;                                     \
    return_type      p ## func func_args;

#define CALL_SHMEM( name ) p##name

#else

#error Unsupported SHMEM wrapping mode

#endif



#include "scorep_shmem_function_list.inc"

/** Artificial root for SHMEM-only experiments without user-code instrumentation */
extern SCOREP_RegionHandle scorep_shmem_region__SHMEM;

/**
 * Register SHMEM functions and initialize data structures
 */
void
scorep_shmem_register_regions();

#endif /* SCOREP_SHMEMWRAP_REG_H */
