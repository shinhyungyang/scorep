/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
#include <SCOREP_AllocMetric.h>

void
SCOREP_AllocMetric_HandleAlloc_fromF08( SCOREP_AllocMetric* allocMetric,
                                        void*               resultAddr,
                                        size_t              size )
{
    SCOREP_AllocMetric_HandleAlloc( allocMetric, ( uint64_t )resultAddr, size );
}
