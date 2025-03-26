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


class OperationMode(Enum):
    read = 'read'
    write = 'write'


def scorep_operation_mode(op_type: OperationMode) -> str:
    return f'SCOREP_IO_OPERATION_MODE_{op_type.value.upper()}'


def scorep_blocking_operation_flag(flag: bool) -> str:
    if flag:
        return 'SCOREP_IO_OPERATION_FLAG_BLOCKING'
    else:
        return 'SCOREP_IO_OPERATION_FLAG_NON_BLOCKING'


def scorep_collective_operation_flag(flag: bool) -> str:
    if flag:
        return 'SCOREP_IO_OPERATION_FLAG_COLLECTIVE'
    else:
        return 'SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE'
