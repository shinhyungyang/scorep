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
This module provides all data necessary to generator individual wrappers.

The wrapgen uses information from three main sources:

1. The :term:`pympistandard` package, which uses the :term:`apis.json` data exported from the MPI standard
2. A :term:`scorep.json` file
3. The template files
"""
from typing import List, Callable
from enum import Enum, auto
import logging

from . import mpistandard as std
from . import scorep as scp
from .mpi_version import is_valid_mpi_version

from .names import MPINamespaceDict

_logger = logging.getLogger(__name__)


class ProcedureData:
    def __init__(self, canonical_name: str, std_procedure: std.Procedure, genes: scp.Genes):
        self._canonical_name = canonical_name
        self._std_procedure = std_procedure
        self._genes = genes

    @property
    def canonical_name(self):
        return self._canonical_name

    @property
    def std_procedure(self):
        return self._std_procedure

    @property
    def genes(self):
        return self._genes


EXCLUDED_PROCEDURES: MPINamespaceDict = MPINamespaceDict()
PROCEDURES: MPINamespaceDict = MPINamespaceDict()


def init():
    _logger.info('Initializing wrapgen')
    scp.init()
    std.init()
    global PROCEDURES
    for name, proc in std.PROCEDURES.items():
        genes = scp.SCOREP_GENOME.get(name, default=scp.Genes())
        canonical_name = PROCEDURES.key_transform(name)
        if is_valid_mpi_version(genes.mpi_version):
            PROCEDURES[canonical_name] = ProcedureData(canonical_name, proc, genes)
        else:
            EXCLUDED_PROCEDURES[canonical_name] = ProcedureData(canonical_name, proc, genes)


class ProcedureSets(Enum):
    ALL = auto()
    ALL_ISO_C = auto()
    ALL_F90 = auto()
    ALL_F08 = auto()
    ALL_F90_C = auto()
    EXCLUDED = auto()


def get_predefined_procedure_set(pset: ProcedureSets) -> List[str]:
    if pset is ProcedureSets.ALL:
        return all_procedure_names_with(lambda k, v: True)
    elif pset is ProcedureSets.ALL_ISO_C:
        return all_procedure_names_with(lambda k, v: v.std_procedure.express.iso_c is not None)
    elif pset is ProcedureSets.ALL_F90:
        return all_procedure_names_with(lambda k, v: v.std_procedure.express.f90 is not None)
    elif pset is ProcedureSets.ALL_F08:
        return all_procedure_names_with(lambda k, v: v.std_procedure.express.f08 is not None)
    elif pset is ProcedureSets.ALL_F90_C:
        return all_procedure_names_with(lambda k, v:
                                        v.std_procedure.express.f90 is not None
                                        and v.std_procedure.express.iso_c is not None)
    elif pset is ProcedureSets.EXCLUDED:
        return list[str](EXCLUDED_PROCEDURES.keys())
    else:
        raise ValueError(f"'{pset}' is not a valid ProcedureSet enumeration value")


def all_procedure_names_with(condition: Callable[[str, ProcedureData], bool]) -> List[str]:
    return [key
            for key, value in PROCEDURES.items()
            if condition(key, value)]


def get_num_procedures_in_set(pset: ProcedureSets) -> int:
    return len(get_predefined_procedure_set(pset))
