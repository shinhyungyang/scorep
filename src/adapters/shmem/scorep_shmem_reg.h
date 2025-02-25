/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014, 2016, 2018, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2017,
 * Forschungszentrum Juelich GmbH, Germany
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
 * - Declaration of Score-P region handles for wrapped SHMEM functions
 * - Declaration of strong function symbols
 */

#define SCOREP_SHMEM_PROCESS_FUNC( type, return_type, func, func_args ) \
    extern SCOREP_RegionHandle scorep_shmem_region__ ## func; \
    return_type func func_args; \
    return_type      p ## func func_args;

#include "scorep_shmem_function_list.inc.c"

/**
 * Register SHMEM functions and initialize data structures
 */
void
scorep_shmem_register_regions( void );

#endif /* SCOREP_SHMEMWRAP_REG_H */
