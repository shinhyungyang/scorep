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
__all__ = ['generator_callback_c_wrapper',
           'generator_callback_c_register_c',
           'generator_callback_c_register_h',
           'generate_c_group_enum',
           'generate_c_enable_derived_groups']

import inspect
from typing import Dict, Type
import logging

from wrapgen.data import ProcedureData
from wrapgen.data import scorep as scp
from wrapgen.data import mpistandard as std
from wrapgen.generator.wrapper_hooks import GeneratorOutput
from wrapgen.generator.wrapper_status import WrapperStatus
from wrapgen.generator.utils_c import generate_wrapper_guard, generate_multiline_comment
from wrapgen.generator.debug_output import is_debug_enabled

from .c_wrapper import Wrapper, DefaultWrapper, FakeFinalizationWrapper, GuardComplianceWrapper, ProtoArgsWrapper
from .scorep_c_procedure import ScorepCProcedure
from wrapgen.generator.c import wrapper_tasks
from wrapgen.generator.c.wrapper_tasks.ctask import CTask
from ... import errors

_logger = logging.getLogger(__name__)

GENERATED_WRAPPERS: Dict[str, WrapperStatus] = {}


def get_public_task_types() -> Dict[str, Type[CTask]]:
    ctask_subclasses = inspect.getmembers(wrapper_tasks,
                                          predicate=lambda m:
                                          inspect.isclass(m)
                                          and m is not CTask
                                          and CTask in inspect.getmro(m)
                                          and not m.is_auxiliary())
    return {c[0]: c[1] for c in ctask_subclasses}


def get_wrapper_types() -> Dict[str, Type[DefaultWrapper]]:
    wrapper_types = inspect.getmembers(c_wrapper,
                                       predicate=lambda m:
                                       inspect.isclass(m)
                                       and DefaultWrapper in inspect.getmro(m))
    # The above returns only classes which inherit from 'DefaultWrapper'
    # noinspection PyTypeChecker
    return {m[0]: m[1] for m in wrapper_types}


def assert_not_seen_before(procedure_data: ProcedureData) -> None:
    if procedure_data.canonical_name in GENERATED_WRAPPERS:
        errors.error(f"'{procedure_data.canonical_name}' has already been generated in the C-layer.", _logger)


def generator_callback_c_wrapper(procedure_data: ProcedureData) -> GeneratorOutput:
    assert_not_seen_before(procedure_data)
    procedure_c = procedure_data.std_procedure.express.iso_c
    if procedure_c is None:
        msg = (f"Requested to generate '{procedure_data.canonical_name}' in C-layer"
               f"but the associated pympistandard procedure does not have an iso_c expression")
        errors.error_if_strict(msg, _logger)
        yield from generate_multiline_comment('WARNING: ', msg)
        GENERATED_WRAPPERS[procedure_data.canonical_name] = WrapperStatus.TODO
        return

    wrapper_generator = make_wrapper_generator(procedure_data.canonical_name, procedure_c, procedure_data.genes)
    yield from wrapper_generator.generate()

    if any([isinstance(task, wrapper_tasks.TaskTodo) for task in wrapper_generator.attached_tasks()]):
        GENERATED_WRAPPERS[procedure_data.canonical_name] = WrapperStatus.TODO
    else:
        GENERATED_WRAPPERS[procedure_data.canonical_name] = WrapperStatus.DONE

    if procedure_data.std_procedure.has_embiggenment():
        yield '\n'
        yield from make_wrapper_generator(procedure_data.canonical_name,
                                          procedure_data.std_procedure.express.embiggen.iso_c,
                                          procedure_data.genes).generate()


def make_wrapper_generator(canonical_name: str, std_procedure: std.ISOCSymbol, genes: scp.Genes) -> Wrapper:
    scp_procedure = ScorepCProcedure(std_procedure=std_procedure,
                                     canonical_name=canonical_name,
                                     version=genes.mpi_version,
                                     **genes.parameters)

    try:
        wrapper_type = get_wrapper_types()[genes.template]
    except KeyError:
        errors.error(f"The wrapper template {genes.template} is not implemented in the C layer", _logger)
    wrapper = wrapper_type(scp_procedure, guards=genes.guards, group=genes.group)
    if isinstance(wrapper, ProtoArgsWrapper) and (genes.c_variants is None or len(genes.c_variants) == 0):
        errors.error_if_strict(f"The wrapper for {canonical_name} uses the 'ProtoArgsWrapper' template,"
                               f" but no prototype variants are defined via 'c_variants'", _logger)

    task_types = get_public_task_types()
    for task_data in genes.tasks:
        if task_data.typename in task_types:
            task_type = task_types[task_data.typename]
            wrapper.add_task(task_type, task_data.attributes)
        else:
            msg = f"The task {task_data.typename} is not implemented in the C layer"
            errors.error_if_strict(msg, _logger)
            wrapper.add_task(wrapper_tasks.TaskTodo, {'message': msg})

    if len(wrapper.attached_tasks()) == 0:
        wrapper.add_task(wrapper_tasks.TaskTodo,
                         {'message': f'Implement more than the default behavior for {canonical_name}'})

    if is_debug_enabled():
        wrapper.set_debug_formatter(lambda msg: '// ' + msg.replace('\n', '') + '\n')
    return wrapper


def generator_callback_c_register_c(procedure_data: ProcedureData) -> GeneratorOutput:
    proc = procedure_data.std_procedure.express.iso_c
    genes = procedure_data.genes
    canonical_name = procedure_data.canonical_name
    if proc is None:
        return
    version = procedure_data.genes.mpi_version
    yield from generate_wrapper_guard(procedure_data.canonical_name, version, proc.name, genes.guards)

    yield f'''\
if ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_{genes.group.upper()} || SCOREP_IsUnwindingEnabled() )
{{
    scorep_mpi_regions[ SCOREP_MPI_REGION__{canonical_name.upper()} ] =
        SCOREP_Definitions_NewRegion( "{proc.name}",
                                      NULL,
                                      file_id,
                                      SCOREP_INVALID_LINE_NO,
                                      SCOREP_INVALID_LINE_NO,
                                      SCOREP_PARADIGM_MPI,
                                      {scp.regions.scorep_region_type(genes.region)} );
}}
#endif

'''


def generator_callback_c_register_h(procedure_data: ProcedureData) -> GeneratorOutput:
    yield f'SCOREP_MPI_REGION__{procedure_data.canonical_name.upper()},\n'


def generate_c_group_enum() -> GeneratorOutput:
    yield '/* pure groups, which can be specified in conf */\n'
    for pure_group in scp.groups.scorep_pure_groups:
        yield f'SCOREP_MPI_ENABLED_{pure_group.upper()} = 1 << {generate_c_group_enum.counter},\n'
        generate_c_group_enum.counter += 1

    yield '/* derived groups, which are a combination of existing groups */\n'
    for derived_group in scp.groups.scorep_derived_groups:
        yield f'SCOREP_MPI_ENABLED_{derived_group[0].upper()}_{derived_group[1].upper()} = 1 << {generate_c_group_enum.counter},\n'
        generate_c_group_enum.counter += 1

    yield f'SCOREP_MPI_ENABLED_REQUEST = 1 << {generate_c_group_enum.counter},\n'
    generate_c_group_enum.counter += 1

    yield '/* ALL comprises all pure groups */\n'
    yield 'SCOREP_MPI_ENABLED_ALL = '
    yield '|\n'.join([f'SCOREP_MPI_ENABLED_{pure_group.upper()}'
                      for pure_group in scp.groups.scorep_all_groups])
    yield ',\n'

    yield '/* DEFAULT groups */\n'
    yield 'SCOREP_MPI_ENABLED_DEFAULT = '
    yield '|\n'.join([f'SCOREP_MPI_ENABLED_{pure_group.upper()}'
                      for pure_group in scp.groups.scorep_default_enabled_groups])


generate_c_group_enum.counter = 0


def generate_c_enable_derived_groups() -> GeneratorOutput:
    for derived_group in scp.groups.scorep_derived_groups:
        yield f'''if ( ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_{derived_group[0].upper()} ) && ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_{derived_group[1].upper()} ) )
    {{
        scorep_mpi_enabled |= SCOREP_MPI_ENABLED_{derived_group[0].upper()}_{derived_group[1].upper()};
    }}
'''
