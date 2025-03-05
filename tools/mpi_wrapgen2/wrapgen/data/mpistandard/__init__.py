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
This module provides an interface to the pympistandard package.

Since more recent versions (at least since 4.0), the MPI standard provides
program-readable information on the MPI interface collected in the
:term:`apis.json` file. This file is generated automatically by a so-called
*binding tool* from the LaTeX sources. From it, the language bindings for C, F90,
F08 and LIS (language independent specific) of all MPI procedures that are
included in the *current standard* can be generated. This includes the
*deprecated*, but not the *removed* interfaces.

However, the information is stored in an abstract and language independent form
and additional knowledge is required to reproduce the bindings. For example, for
each argument of an MPI procedure a *kind* property (not to be confused with
the Fortran KIND) is stored. To get the C bindings, a map from *kind* s to C
types is required.

The :term:`pympistandard` package provides a python API to this derived
information. It needs an :term:`apis.json` file as input.

Versions of the MPI standard
----------------------------

The :term:`apis.json` file contains only information on the **current** version
of the standard. This includes the deprecated interfaces, but not the removed
interfaces.

The Standard allows an MPI implementation to provide removed interfaces for
backwards compatibility. Applications can potentially still rely upon this
functionality and the wrapper has to intercept the removed procedures.

Therefore, this module contains multiple :term:`apis.json` files from
multiple MPI versions and merges them into a single :term:`apis.json` to
initialize the :term:`pympistandard` package. Only the *most recent* entry for
each procedure remains in the merged :term:`apis.json`.

* Versions of the Standard prior to 4.0 did not provide an :term:`apis.json`.
  The (manually written) files for these versions only include the data for
  interfaces that were removed in a later version.
  In MPI 3.0 some procedures from the MPI 1.0 standard were removed. Therefore,
  the file for version 2.2 includes all those interfaces, but nothing more.
* We expect that future versions of the Standard will provide a compatible
  :term:`apis.json` file. For each version from 4.0 upwards, the entire
  :term:`apis.json` file is stored.

"""
__all__ = ['init',
           'PROCEDURES',
           'Procedure',
           'Parameter', 'Direction',
           'ISOCSymbol', 'EmbiggenedISOCSymbol', 'ISOCParameter', 'EmbiggenedISOCParameter',
           'F90Symbol', 'F90Parameter',
           'F08Symbol', 'EmbiggenedF08Procedure', 'F08ParameterBase', 'F08Parameter']

import json
import logging
import os
from typing import Optional, Tuple, List

import pympistandard
from pympistandard import PROCEDURES
from pympistandard.f08 import F08Symbol, EmbiggenedF08Procedure, F08ParameterBase, F08Parameter
from pympistandard.f90 import F90Symbol, F90Parameter
from pympistandard.isoc import ISOCSymbol, EmbiggenedISOCSymbol, ISOCParameter, EmbiggenedISOCParameter
from pympistandard.parameter import Parameter, Direction
from pympistandard.procedure import Procedure

from wrapgen.data.mpi_version import Version, is_valid_mpi_version
from wrapgen import errors
from . import kinds_mpi1

_logger = logging.getLogger(__name__)


def init() -> None:
    merge_api_files()

    # Use the 'apis.json' provided with this module
    _logger.info("Initializing the mpistandard module with %s", merged_api_file_path())
    os.environ.update({'MPISTANDARD': get_module_path()})
    pympistandard.use_api_version(1)

    # Register additional Kinds
    for key, item in kinds_mpi1.__dict__.items():
        if isinstance(item, pympistandard.Kind):
            pympistandard.KINDS[key.lower()] = item


def merged_api_file_path() -> str:
    return os.path.join(get_module_path(), 'apis.json')


def get_module_path() -> str:
    return os.path.dirname(os.path.abspath(__file__))


def _parse_version(string: str) -> Optional[Version]:
    version = Version(string.split('_'))
    if is_valid_mpi_version(version) and version.has_minor():
        return version
    return None


def merge_api_files() -> None:
    """
    Produce one apis.json file from all api files of the form 'X_Y.json' in
    api_versions (where X is the MPI major version and Y is the MPI minor
    version).

    Entries from newer versions overwrite entries from older versions of MPI.
    The data for each procedure is augmented by the additional key
    _latest_mpi_version showing the newest MPI standard that this procedure is
    found in.
    """
    api_versions_path = os.path.join(get_module_path(), 'api_versions')

    _logger.info('Looking for API files in %s', api_versions_path)
    filename_and_version: List[Tuple[str, Version]] = []
    for file_name in os.listdir(api_versions_path):
        basename, ext = os.path.splitext(file_name)
        if ext != '.json':
            _logger.warning('Skipping file %s without .json extension', file_name)
            continue
        version = _parse_version(basename)
        if version is None:
            errors.error(f"Could not extract version info from file name {file_name}", _logger)
        filename_and_version.append((file_name, version))
    # Sort by version
    filename_and_version = sorted(filename_and_version, key=lambda x: x[1])
    _logger.info('Found API files for MPI versions %s', [version for name, version in filename_and_version])

    _logger.info('Merging API data')
    api_all_versions = {}
    for file_name, version in filename_and_version:
        _logger.debug('Loading API data for MPI version %s from %s', version, file_name)
        try:
            with open(file=os.path.join(api_versions_path, file_name), mode='r') as infile:
                api = json.load(infile)
                for procedure_name, procedure_data in api.items():
                    procedure_data.update({'_latest_mpi_version': version.tuple()})
                api_all_versions.update(api)
        except OSError:
            errors.error(f"The file '{file_name}' is not readable", _logger)
        except json.JSONDecodeError:
            errors.error(f"The file '{file_name}' does not contain valid json data.", _logger)

    _logger.info('Writing the merged API data to %s', merged_api_file_path())
    with open(merged_api_file_path(), mode='w') as outfile:
        json.dump(api_all_versions, outfile, indent=4, sort_keys=True)
