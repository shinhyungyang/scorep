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
Provide Score-P specific information on MPI wrappers. This includes per-procedure
data as well as general data.

To use this module, the :func:`init()` function has to be called explicitly,
which reads procedure-specific data from a .json file and makes it available as
the dictionary :data:`SCOREP_GENOME`.
"""
__all__ = ['init',
           'SCOREP_GENOME', 'ScorepGenomeError', 'Genes', 'groups', 'collectives',
           'Version', 'regions']

import json
import logging
import os
from typing import Dict, Callable

from . import groups, collectives, regions
from .genes import Genes
from ..mpi_version import Version
from ..names import is_canonical, MPINamespaceDict
from wrapgen import errors

_logger = logging.getLogger(__name__)

SCOREP_GENOME: MPINamespaceDict = MPINamespaceDict()
"""Mapping from procedure names to :class:`Genes`"""


class ScorepGenomeError(Exception):
    pass


def init() -> None:
    _logger.info("Loading Score-P data from file %s.", get_scorep_file())
    scorep_raw_data = load_data_from_json(get_scorep_file())
    global SCOREP_GENOME
    for key, raw_procedure_data in scorep_raw_data.items():
        if not is_canonical(key):
            raise ScorepGenomeError("Key '%s' is not in canonical form", key)
        if key in SCOREP_GENOME:
            raise ScorepGenomeError("Key '%s' is ambiguous", key)
        procedure_genes = Genes.from_raw(raw_procedure_data)
        SCOREP_GENOME[key] = procedure_genes


def load_data_from_json(file_name: str) -> Dict:
    if os.path.isfile(file_name):
        with open(file_name, 'r') as file:
            try:
                data = json.load(file)
            except json.JSONDecodeError as ex:
                errors.log_and_raise(ex, msg=f"The file '{file_name}' does not contain valid json data.",
                                     logger=_logger)
        return data
    else:
        raise FileNotFoundError(file_name)


def get_scorep_file() -> str:
    return os.path.join(get_module_path(), 'scorep.json')


def get_module_path() -> str:
    return os.path.dirname(os.path.abspath(__file__))


def update_json_data(update_function: Callable[[Dict], Dict]) -> None:
    """
    **For maintenance only!**: Load raw data from scorep.json, modify it with
    the provided update function and overwrite the file with the updated data.

    :param update_function: Function that updates the raw data.
    """
    data = load_data_from_json(get_scorep_file())
    updated_data = update_function(data)

    tmp_data_path: str = os.path.join(get_module_path(), 'tmp-scorep.json')
    try:
        with open(tmp_data_path, mode='w') as file:
            json.dump(updated_data, file, sort_keys=True, indent=4)
    except json.JSONDecodeError as ex:
        errors.log_and_raise(ex, msg='Could not serialize JSON', logger=_logger)
    else:
        os.replace(tmp_data_path, get_scorep_file())
    finally:
        global SCOREP_GENOME
        SCOREP_GENOME.clear()
