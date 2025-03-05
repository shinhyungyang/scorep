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
from enum import Enum


class CompletionType(Enum):
    separate = 'separate'
    combined = 'combined'


def scorep_completion_type(completion_type: CompletionType) -> str:
    return f'SCOREP_MPI_RMA_REQUEST_{completion_type.value.upper()}_COMPLETION'


class AtomicType(Enum):
    accumulate = 'accumulate'
    increment = 'increment'
    test_and_set = 'test_and_set'
    compare_and_swap = 'compare_and_swap'
    swap = 'swap'
    fetch_and_add = 'fetch_and_add'
    fetch_and_increment = 'fetch_and_increment'
    add = 'add'
    fetch_and_accumulate = 'fetch_and_accumulate'


def scorep_atomic_type(atomic_type: AtomicType) -> str:
    return f'SCOREP_RMA_ATOMIC_TYPE_{atomic_type.value.upper()}'


class RequestCallback(Enum):
    scorep_mpi_rma_request_write_full_completion = 'scorep_mpi_rma_request_write_full_completion'
    scorep_mpi_rma_request_write_standard_completion = 'scorep_mpi_rma_request_write_standard_completion'
    scorep_mpi_rma_request_write_standard_completion_and_remove = 'scorep_mpi_rma_request_write_standard_completion_and_remove'


class EpochType(Enum):
    access = 'access'
    exposure = 'exposure'


def scorep_epoch_type(epoch_type: EpochType) -> str:
    return f'SCOREP_MPI_RMA_{epoch_type.value.upper()}_EPOCH'
