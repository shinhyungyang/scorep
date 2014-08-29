/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2014,
 * Technische Universitaet, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file scorep_monolithic_subsystems.c
 */


#include <config.h>

#include <SCOREP_Subsystem.h>

extern const SCOREP_Subsystem SCOREP_Subsystem_UserAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_MpiAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_ShmemAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_PompUserAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_PompOmpAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_CompilerAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_MetricService;
extern const SCOREP_Subsystem SCOREP_Subsystem_CudaAdapter;
extern const SCOREP_Subsystem SCOREP_Subsystem_ThreadForkJoin;

/** @brief a NULL terminated list of linked in subsystems. */
const SCOREP_Subsystem* scorep_subsystems[] = {
    &SCOREP_Subsystem_MetricService,
    &SCOREP_Subsystem_CompilerAdapter,
    &SCOREP_Subsystem_UserAdapter,
#ifdef SCOREP_SUBSYSTEMS_OMP
    &SCOREP_Subsystem_PompUserAdapter,
    &SCOREP_Subsystem_PompOmpAdapter,
#endif
#ifdef SCOREP_SUBSYSTEMS_MPI
    &SCOREP_Subsystem_MpiAdapter,
#endif
#ifdef SCOREP_SUBSYSTEMS_SHMEM
    &SCOREP_Subsystem_ShmemAdapter,
#endif
#if HAVE_CUDA_SUPPORT
    &SCOREP_Subsystem_CudaAdapter,
#endif
    &SCOREP_Subsystem_ThreadForkJoin
};

const size_t scorep_number_of_subsystems = sizeof( scorep_subsystems ) /
                                           sizeof( scorep_subsystems[ 0 ] );
