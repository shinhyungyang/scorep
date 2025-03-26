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
from typing import Iterable

from wrapgen.data.names import remove_prefix
from wrapgen.data.mpi_version import Version

from .utils import generate_lines
from .wrapper_hooks import GeneratorOutput


def confdef_pmpi_symbol(name: str, version: Version) -> str:
    return f'MPI_{version.major}_{version.minor}_SYMBOL_PMPI_{remove_prefix(name).upper()}'


def generate_wrapper_guard(name: str, version: Version, procedure_name: str, guards: Iterable[str]) -> GeneratorOutput:
    yield f'#if HAVE( {confdef_pmpi_symbol(name, version)} )'
    for guard in guards:
        yield f' && !defined(SCOREP_MPI_NO_{guard.upper()})'
    yield f' && !defined({procedure_name})\n'


def generate_multiline_comment(*msg: str) -> GeneratorOutput:
    yield '/*\n'
    for line in generate_lines(*msg):
        yield f' * {line}'
    yield '\n*/\n'
