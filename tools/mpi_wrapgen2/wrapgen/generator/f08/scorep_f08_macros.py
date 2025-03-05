#
# This file is part of the Score-P software (http://www.score-p.org)
#
# Copyright (c) 2025,
# Forschungszentrum Juelich GmbH, Germany
#
# This software may be modified and distributed under the terms of
# a BSD-style license. See the COPYING file in the package base
# directory for details.
#
"""
Functions in this file correspond to SCOREP macros.

On the F08 side, we let the wrapgen expand these directly in the wrappers.
"""
from typing import Any


#
# Macros from
# config.h
#
def HAVE(name: str) -> str:
    """
    Write out the HAVE macro explicitly, because the Fortran preprocessor cannot handle this syntax.
    """
    return f'( defined( HAVE_{name} ) && HAVE_{name} )'


#
# Macros from
# SCOREP_InMeasurement.h
#

def SCOREP_IS_MEASUREMENT_PHASE(phase) -> str:
    return f'(logical(scorep_is_measurement_phase(SCOREP_MEASUREMENT_PHASE_{phase}_C)))'


#
# Macros from
# scorep_mpi_c.h
#

def SCOREP_MPI_IS_EVENT_GEN_ON() -> str:
    return 'logical(scorep_mpi_is_event_gen_on())'


def SCOREP_MPI_IS_EVENT_GEN_ON_FOR(group: str) -> str:
    return f'(logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_{group.upper()}) .gt. 0))'


def SCOREP_MPI_EVENT_GEN_ON() -> str:
    return 'call scorep_mpi_event_gen_on()'


def SCOREP_MPI_EVENT_GEN_OFF() -> str:
    return 'call scorep_mpi_event_gen_off()'


#
# Macros from
# scorep_mpi_communicator_mgmt.h
#

def scorep_mpi_comm_world_handle() -> str:
    return 'scorep_mpi_world%handle'


#
# Macros from
# SCOREP_RuntimeManagement.h
#

def SCOREP_IsUnwindingEnabled() -> str:
    return '(scorep_is_unwinding_enabled)'


#
# Macros from
# Utils_Error.h
#
def UTILS_WARNING(*msg: Any) -> str:
    message = ", &\n".join([f'"{m}"' for m in msg])
    return f'print *, "[Score-P] Warning: ", &\n {message}\n'


def UTILS_FATAL(*msg: Any) -> str:
    message = ", &\n".join([f'"{m}"' for m in msg])
    return f'print *, "[Score-P] Fatal: ", &\n {message}\nERROR STOP\n'
