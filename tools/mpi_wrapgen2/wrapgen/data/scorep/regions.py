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


class RegionType(Enum):
    """
    Only those region types that appear in SCOREP_Mpi_C_Reg.c
    """
    allocate = 'allocate'
    atomic = 'atomic'
    barrier = 'barrier'
    coll_all2all = 'coll_all2all'
    coll_all2one = 'coll_all2one'
    coll_one2all = 'coll_one2all'
    coll_other = 'coll_other'
    deallocate = 'deallocate'
    file_io = 'file_io'
    file_io_metadata = 'file_io_metadata'
    none = 'none'
    point2point = 'point2point'
    rma = 'rma'


def scorep_region_type(region_type: RegionType) -> str:
    return f'SCOREP_REGION_{region_type.value.upper()}'
