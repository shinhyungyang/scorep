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
#include <config.h>
#include <SCOREP_Types.h>
#include <SCOREP_InMeasurement.h>

#include <stdbool.h>

/**
 * Access and modify variables from SCOREP_InMeasurement.h that cannot be
 * accessed directly via bind(c) from Fortran:
 * - scorep_measurement_phase is volatile
 * - scorep_in_measurement is a TLS variable
 *
 * For now, we accept that using the F08 wrappers together with
 * the sampling subsystems might lead to the functions below
 * appearing in the sampling output.
 */

bool
scorep_is_measurement_phase_fromF08( SCOREP_MeasurementPhase phase )
{
    return scorep_measurement_phase == phase;
}

void
scorep_in_measurement_increment_fromF08( void )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
}

void
scorep_in_measurement_decrement_fromF08( void )
{
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

void
scorep_enter_wrapped_region_fromF08( int* inMeasurementSave )
{
    SCOREP_ENTER_WRAPPED_REGION_( *inMeasurementSave );
}

void
scorep_exit_wrapped_region_fromF08( const int* inMeasurementSave )
{
    SCOREP_EXIT_WRAPPED_REGION_( *inMeasurementSave );
}
