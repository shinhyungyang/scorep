/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2011,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_POMP_REGION_INFO_H
#define SCOREP_POMP_REGION_INFO_H

/**
 * @file
 * @ingroup    POMP
 *
 * @brief Declares functionality for interpretation of pomp region strings.
 */

#include <stdbool.h>
#include "SCOREP_Types.h"
#include "SCOREP_Pomp_Lock.h"

/**
 * SCOREP_Pomp_RegionType
 * @ingroup POMP
 * @{
 *
 */
typedef enum /* SCOREP_Pomp_RegionType */
{
    /* Entries must be in same order like scorep_pomp_region_type_map to allow lookup. */
    SCOREP_Pomp_Atomic = 0,
    SCOREP_Pomp_Barrier,
    SCOREP_Pomp_Critical,
    SCOREP_Pomp_Do,
    SCOREP_Pomp_Flush,
    SCOREP_Pomp_For,
    SCOREP_Pomp_Master,
    SCOREP_Pomp_Ordered,
    SCOREP_Pomp_Parallel,
    SCOREP_Pomp_ParallelDo,
    SCOREP_Pomp_ParallelFor,
    SCOREP_Pomp_ParallelSections,
    SCOREP_Pomp_ParallelWorkshare,
    SCOREP_Pomp_UserRegion,
    SCOREP_Pomp_Sections,
    SCOREP_Pomp_Single,
    SCOREP_Pomp_Task,
    SCOREP_Pomp_Taskwait,
    SCOREP_Pomp_Workshare,
    SCOREP_Pomp_NoType
} SCOREP_Pomp_RegionType;


/** Struct which contains all data for a pomp region. */
typedef struct
{
    SCOREP_Pomp_RegionType regionType;        /* region type of construct              */
    char*                  name;              /* critical or user region name          */
    uint32_t               numSections;       /* sections only: number of sections     */
    /* For combined statements (parallel sections, parallel for) we need up to
       four SCOREP regions. So we use a pair of SCOREP regions for the parallel
       statements and a pair of regions for other statements.
       We must reserve space for 3 region handles, because the parallel sections
       construct uses three region handles:
         1. parallel region (parallel begin/end events)
         2. enclosing sections construct (sections enter/exit)
         3. individual sections begin/end                                              */
    SCOREP_RegionHandle innerParallel;        /* SCOREP handle for the inner parallel region */
    SCOREP_RegionHandle barrier;              /* SCOREP handle for the (implicit barrier)    */
    SCOREP_RegionHandle outerBlock;           /* SCOREP handle for the enclosing region      */
    SCOREP_RegionHandle innerBlock;           /* SCOREP handle for the enclosed region       */
    SCOREP_PompLock*    lock;

    char*               startFileName;    /* File containing opening statement         */
    int32_t             startLine1;       /* First line of the opening statement       */
    int32_t             startLine2;       /* Last line of the opening statement        */

    char*               endFileName;      /* File containing the closing statement     */
    int32_t             endLine1;         /* First line of the closing statement       */
    int32_t             endLine2;         /* Last line of the closing statement        */
} SCOREP_Pomp_Region;


/**
 * SCOREP_Pomp_ParseInitString() fills the SCOREP_Pomp_Region object with data read
 * from the @a initString. If the initString does not comply with the
 * specification, the program aborts with exit code 1. @n Rationale:
 * SCOREP_Pomp_ParseInitString() is used during initialization of the measurement
 * system. If an error occurs, it is better to abort than to struggle with
 * undefined behaviour or @e guessing the meaning of the broken string.
 *
 * @note Can be called from multiple threads concurrently, assuming malloc is
 * thread-safe.
 *
 * @note SCOREP_Pomp_ParseInitString() will assign memory to the members of @e
 * region.
 *
 * @param initString A string in the format
 * "length*key=value*[key=value]*". The length field is parsed but not used by
 * this implementation. Possible values for key are listed in
 * scorep_pomp_token_map. The string must at least contain values for the keys @c
 * regionType, @c sscl and @c escl. Possible values for the key @c regionType
 * are listed in @ref scorep_pomp_region_type_map. The format for @c sscl resp.
 * @c escl values
 * is @c "filename:lineNo1:lineNo2".
 *
 * @param region must be a valid object
 *
 * @post At least the required attributes (see SCOREP_Pomp_Region) are set. @n
 * All other members of @e region are set to 0 resp. false.
 * @n If @c regionType=sections than
 * SCOREP_Pomp_Region::numSections has a value > 0. @n If @c regionType=region
 * than SCOREP_Pomp_Region::regionName has a value != 0. @n If @c
 * regionType=critical than SCOREP_Pomp_RegionInfo::regionName may have a value
 * != 0.
 *
 */
void
SCOREP_Pomp_ParseInitString( const char          initString[],
                             SCOREP_Pomp_Region* region );

/** @} */

#endif /* SCOREP_POMP_REGION_INFO_H */
