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
__all__ = ['generator_callback_ac_f08_symbol_check',
           'generator_callback_ac_f08_list_entry_for_symbol_check',
           'generator_callback_ac_f08_linkcheck',
           'generator_callback_ac_c_compliance_check',
           'generate_ac_c_symbol_checks']

from typing import Dict, List
import copy
import re
import logging

from wrapgen.data import ProcedureData
from wrapgen.data import PROCEDURES
from wrapgen.data import mpistandard as std
from wrapgen.data.names import remove_prefix
from wrapgen.data.mpi_version import mpi_standard_versions, is_valid_mpi_version
from wrapgen.language.pympistandard_to_f import make_fortran_argument_descriptor

from .wrapper_hooks import GeneratorOutput
from .utils import generate_lines
from ..language.pympistandard_to_c import make_c_parameter

_logger = logging.getLogger(__name__)


def generator_callback_ac_f08_symbol_check(procedure_data: ProcedureData) -> GeneratorOutput:
    canonical_name = procedure_data.canonical_name
    procedure_f08 = procedure_data.std_procedure.express.f08
    if procedure_f08 is None:
        return
    yield from _generate_ac_f08_symbol_check_inner(canonical_name, procedure_f08)

    if procedure_data.std_procedure.has_embiggenment():
        yield from _generate_ac_f08_symbol_check_inner(canonical_name,
                                                       procedure_data.std_procedure.express.embiggen.f08)


def _generate_ac_f08_symbol_check_inner(canonical_name: str, procedure_f08: std.F08Symbol) -> GeneratorOutput:
    name = canonical_name
    if isinstance(procedure_f08, std.EmbiggenedF08Procedure):
        name += '_c'
    m4_name = f'_MPI_F08_{_no_prefix_upper(name)}'
    yield from _generate_m4_define(m4_name, _generate_compile_check_program(procedure_f08))


def _generate_m4_define(macro_name: str, generate_definition: GeneratorOutput) -> GeneratorOutput:
    yield f'm4_define([{macro_name}], '
    yield '[['
    yield from generate_definition
    yield ']]\n'
    yield f')dnl {macro_name}\n\n'


def _no_prefix_upper(name: str) -> str:
    return remove_prefix(name).upper()


def _shorten_param_names(procedure_f08: std.F08Symbol, string: str) -> str:
    def letter(i: int) -> str:
        return chr(ord('a') + i)

    full_to_short_names = {param.name: f'{letter(i)}_' for i, param in enumerate(procedure_f08.parameters)}

    short = copy.copy(string)
    for full_name, short_name in full_to_short_names.items():
        pattern = f'\\b{full_name}\\b'
        short = re.sub(pattern=pattern, repl=short_name, string=short, flags=re.IGNORECASE)
    return short


def _has_choice_buffer(procedure_f08: std.F08Symbol) -> bool:
    # Does the procedure have choice buffer arguments ?
    for param in procedure_f08.parameters:
        descriptor = make_fortran_argument_descriptor(param)
        if descriptor.type.subtype == '*' and descriptor.dimension == ['..']:
            return True
    return False


def _generate_compile_check_program(procedure_f08: std.F08Symbol) -> GeneratorOutput:
    # Shorten parameter names to a single letter and underscore
    # This solves two problems:
    # 1. It shortens the procedure signature and call. Most Fortran compilers truncate input after 132 chars,
    #    which happens for a lot of MPI procedures
    # 2. Avoid collisions with the m4/autoconf keyword 'index'.
    #    Appearance of the word 'index' in the test program leads to failing of autoconf.
    #    Some MPI procedures have 'index' as an argument name
    #
    # We have to make sure to substitute parameter names everywhere:
    # 1. In the dummy argument list
    # 2. In the dummy argument declarations
    #    Take care: argument names can be part of declarations for other arguments too:
    #               E.g. integer :: count
    #                    type(mpi_status) :: array_of_statuses(count)
    # 3. In the call to the MPI procedure

    yield from (
        'subroutine check_f08_sym(',
        ','.join([_shorten_param_names(procedure_f08, param.name) for param in procedure_f08.parameters]),
        ')\n'
    )

    yield from (
        f'use :: mpi_f08, this_procedure => {procedure_f08.name},&\n',
        f'&profiling_procedure => P{procedure_f08.name}\n',
        'implicit none\n',
        '\n',
    )

    # Declare all variables needed for our call
    #   - Group variables that share the same descriptor
    #   - Shorten variable names
    #   - Replace any type(*),dimension(..) with a custom type(mytype):
    #     This achieves two things:
    #     1. Works for compilers that do not support the F08 TS features
    #     2. Prevents successful compilation when no generic interface is available but type-specific routines are found
    #        in the mpi_f08 module
    #        This is an issue with MPI_Sizeof and declaring the buffer as 'real,dimension(*)': This compiles, but the
    #        configure check then reports a wrong name for the  MPI routine (the type-specific 'mpi_sizeof_real_32'
    #        instead of a generic mpi_sizeof_f08 (which does not exist))
    if _has_choice_buffer(procedure_f08):
        yield from (
            'type mytype\n',
            'integer :: value\n',
            'end type\n\n'
        )

    groupings: Dict[str, List[str]] = {}
    for param in procedure_f08.parameters:
        descriptor = make_fortran_argument_descriptor(param)
        if descriptor.type.subtype == '*' and descriptor.dimension == ['..']:
            descriptor.type.name = 'type'
            descriptor.type.subtype = 'mytype'
            descriptor.dimension = ['*']
        descriptor_str = _shorten_param_names(procedure_f08, f'{descriptor}')
        if descriptor_str not in groupings:
            groupings[descriptor_str] = []
        groupings[descriptor_str].append(_shorten_param_names(procedure_f08, f'{param.name}'))
    for desc, params in groupings.items():
        yield f'{desc} :: {", ".join(params)}\n'
    yield '\n'

    if procedure_f08.return_type is not None:
        yield f'{procedure_f08.return_type} :: res_\n'

    if procedure_f08.return_type is None:
        yield 'call '
    else:
        yield 'res_ = '
    yield from (
        f'this_procedure(',
        ','.join([_shorten_param_names(procedure_f08, p.name) for p in procedure_f08.parameters]),
        ')\n'
    )

    yield 'end subroutine'


def generator_callback_ac_f08_linkcheck(procedure_data: ProcedureData) -> GeneratorOutput:
    canonical_name = procedure_data.canonical_name
    procedure_f08 = procedure_data.std_procedure.express.f08
    if procedure_f08 is None:
        return
    yield from _generate_f08_linkcheck_inner(canonical_name, procedure_f08)

    if procedure_data.std_procedure.has_embiggenment():
        yield from _generate_f08_linkcheck_inner(canonical_name, procedure_data.std_procedure.express.embiggen.f08)


def _generate_f08_linkcheck_inner(canonical_name: str, procedure_f08: std.F08Symbol) -> GeneratorOutput:
    name = canonical_name
    if isinstance(procedure_f08, std.EmbiggenedF08Procedure):
        name += '_c'
    m4_name = f'_MPI_F08_LINKCHECK_{_no_prefix_upper(name)}'
    yield from _generate_m4_define(m4_name, _generate_f08_linkcheck_program(procedure_f08))


def _generate_f08_linkcheck_program(procedure_f08: std.F08Symbol) -> GeneratorOutput:
    yield """program main
use :: mpi_f08
implicit none
"""

    if _has_choice_buffer(procedure_f08):
        yield from (
            'type mytype\n',
            'integer :: value\n',
            'end type\n\n'
        )

    groupings: Dict[str, List[str]] = {}
    for param in procedure_f08.parameters:
        descriptor = make_fortran_argument_descriptor(param)
        if descriptor.type.subtype == '*' and descriptor.dimension == ['..']:
            descriptor.type.name = 'type'
            descriptor.type.subtype = 'mytype'
            descriptor.dimension = ['10']
        for k in descriptor.type.parameters:
            if k.upper() == 'LEN':
                descriptor.type.parameters[k] = '10'
        descriptor.attributes = [a for a in descriptor.attributes if a.upper() != 'OPTIONAL']
        descriptor.intent = None
        descriptor_str = _shorten_param_names(procedure_f08, f'{descriptor}')
        if descriptor_str not in groupings:
            groupings[descriptor_str] = []
        groupings[descriptor_str].append(_shorten_param_names(procedure_f08, f'{param.name}'))
    for desc, params in groupings.items():
        yield f'{desc} :: {", ".join(params)}\n'
    yield '\n'

    yield f'call P{procedure_f08.name}('
    yield ','.join([_shorten_param_names(procedure_f08, p.name) for p in procedure_f08.parameters])
    yield ')\n'

    yield 'end program\n'


def generator_callback_ac_f08_list_entry_for_symbol_check(procedure_data: ProcedureData) -> GeneratorOutput:
    procedure_f08 = procedure_data.std_procedure.express.f08
    if procedure_f08 is None:
        return
    if not generator_callback_ac_f08_list_entry_for_symbol_check.first:
        yield ',\n    '
    yield _no_prefix_upper(procedure_data.canonical_name)
    generator_callback_ac_f08_list_entry_for_symbol_check.first = False

    if procedure_data.std_procedure.has_embiggenment():
        yield ',\n    '
        yield _no_prefix_upper(procedure_data.canonical_name + '_C')


generator_callback_ac_f08_list_entry_for_symbol_check.first = True


def generator_callback_ac_c_compliance_check(procedure_data: ProcedureData) -> GeneratorOutput:
    variants = procedure_data.genes.c_variants
    if variants is None or len(variants) == 0:
        return

    procedure_c = procedure_data.std_procedure.express.iso_c
    if procedure_c is None:
        return

    yield '_SCOREP_MPI_CHECK_COMPLIANCE(\n'
    yield '[int], [return 0],\n'
    c_parameters = [make_c_parameter(p) for p in procedure_c.parameters]
    parameters = ', '.join([f'{"const " if p.const else ""}{p.type()}, {p.name}' for p in c_parameters])
    yield f'[{procedure_c.name}], [{parameters}],\n'

    variant_strings = []
    for variant in variants:
        variant_name = variant['name']
        variant_args = variant['args']
        variant_strings.append(f'[{variant_name}], [{variant_args}]')
    yield ',\n'.join(variant_strings)
    yield ')\n\n'


def _is_handle_conversion(procedure_data: ProcedureData):
    return re.match(r'\w+_(f2(c|f08)|c2(f|f08)|f082[cf])', procedure_data.canonical_name) is not None


def generate_ac_c_symbol_checks() -> GeneratorOutput:
    subgroups = ('', 'large', 'handle conversion')
    groups = {(version, sg): list() for version in mpi_standard_versions for sg in subgroups}
    for name, procedure_data in PROCEDURES.items():
        genes = procedure_data.genes
        for proc, version, sg in ((procedure_data.std_procedure.express.iso_c, genes.mpi_version, ''),
                                  (procedure_data.std_procedure.express.embiggen.iso_c, genes.large_mpi_version,
                                   'large')):
            if proc is not None and is_valid_mpi_version(version):
                if _is_handle_conversion(procedure_data):
                    sg = 'handle conversion'
                groups[(version, sg)].append(f'P{proc.name}')

    for group, names in groups.items():
        if len(names) > 0:
            yield f'## MPI {group[0]} {group[1]} functions\n'
            yield f'SCOREP_CHECK_SYMBOLS([MPI {group[0]}], [], [],\n   ['
            yield ',\n    '.join(names)
            yield '])\n\n\n'


def generate_multiline_comment(*msg: str) -> GeneratorOutput:
    for line in generate_lines(*msg):
        yield from ('## ', line)
    yield '\n'
