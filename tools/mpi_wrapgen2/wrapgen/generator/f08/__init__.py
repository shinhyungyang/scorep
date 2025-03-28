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
__all__ = ['generator_callback_f08_wrapper',
           'generator_callback_f08_register',
           'generate_f08_group_params']

from typing import Dict, Type
import inspect
import logging

from wrapgen.data import ProcedureData
from wrapgen.data import scorep as scp
from wrapgen.data import mpistandard as std
from wrapgen.generator.wrapper_status import WrapperStatus
from wrapgen.generator.utils_f import generate_multiline_comment
from wrapgen.generator.wrapper_hooks import GeneratorOutput
from wrapgen.generator.debug_output import is_debug_enabled

from . import scorep_f08_macros
from .scorep_f08_procedure import ScorepF08Procedure
from .f08_wrapper import Wrapper, DefaultWrapper, FakeFinalizationWrapper
from wrapgen.generator.f08 import wrapper_tasks
from wrapgen.generator.f08.wrapper_tasks import F08Task
from ... import errors

_logger = logging.getLogger(__name__)

GENERATED_WRAPPERS: Dict[str, WrapperStatus] = {}


def get_public_task_types() -> Dict[str, Type[F08Task]]:
    f08task_subclasses = inspect.getmembers(wrapper_tasks,
                                            predicate=lambda m:
                                            inspect.isclass(m)
                                            and m is not F08Task
                                            and F08Task in inspect.getmro(m)
                                            and not m.is_auxiliary())
    return {c[0]: c[1] for c in f08task_subclasses}


def get_wrapper_types() -> Dict[str, Type[DefaultWrapper]]:
    wrapper_types = inspect.getmembers(f08_wrapper,
                                       predicate=lambda m:
                                       inspect.isclass(m)
                                       and DefaultWrapper in inspect.getmro(m))
    # The above returns only classes which inherit from 'DefaultWrapper'
    # noinspection PyTypeChecker
    return {m[0]: m[1] for m in wrapper_types}


def assert_not_seen_before(procedure_data: ProcedureData) -> None:
    if procedure_data.canonical_name in GENERATED_WRAPPERS:
        errors.error(f"'{procedure_data.canonical_name}' has already been generated in the F08-layer.", _logger)


def generator_callback_f08_wrapper(procedure_data: ProcedureData) -> GeneratorOutput:
    assert_not_seen_before(procedure_data)
    procedure_f08 = procedure_data.std_procedure.express.f08
    canonical_name = procedure_data.canonical_name
    if procedure_f08 is None:
        msg = (f"Requested to generate '{canonical_name}' in F08-layer"
               f"but the associated pympistandard procedure does not have an f08 expression")
        errors.error_if_strict(msg, _logger)
        yield from generate_multiline_comment('WARNING: ', msg)
        GENERATED_WRAPPERS[canonical_name] = WrapperStatus.TODO
        return

    wrapper_generator = make_wrapper_generator(procedure_data.canonical_name, procedure_f08, procedure_data.genes)
    yield from wrapper_generator.generate()

    if any([isinstance(task, wrapper_tasks.TaskTodo) for task in wrapper_generator.attached_tasks()]):
        GENERATED_WRAPPERS[canonical_name] = WrapperStatus.TODO
    else:
        GENERATED_WRAPPERS[canonical_name] = WrapperStatus.DONE

    if procedure_data.std_procedure.has_embiggenment():
        yield '\n'
        yield from make_wrapper_generator(canonical_name, procedure_data.std_procedure.express.embiggen.f08,
                                          procedure_data.genes).generate()


def make_wrapper_generator(canonical_name: str, std_procedure: std.F08Symbol, genes: scp.Genes) -> Wrapper:
    scp_procedure = ScorepF08Procedure(std_procedure=std_procedure,
                                       canonical_name=canonical_name,
                                       version=genes.mpi_version,
                                       **genes.parameters)

    try:
        wrapper_type = get_wrapper_types()[genes.template]
    except KeyError:
        errors.error(f"The wrapper template {genes.template} is not implemented in the F08 layer", _logger)
    wrapper = wrapper_type(scp_procedure, guards=genes.guards, group=genes.group)

    task_types = get_public_task_types()
    for task_data in genes.tasks:
        if task_data.typename in task_types:
            task_type = task_types[task_data.typename]
            wrapper.add_task(task_type, task_data.attributes)
        else:
            msg = f"The task {task_data.typename} is not implemented in the F08 layer"
            errors.error_if_strict(msg, _logger)
            wrapper.add_task(wrapper_tasks.TaskTodo, {'message': msg})

    if len(wrapper.attached_tasks()) == 0:
        wrapper.add_task(wrapper_tasks.TaskTodo,
                         {'message': f'Implement more than the default behavior for {canonical_name}'})

    if is_debug_enabled():
        wrapper.set_debug_formatter(lambda msg: f'! {msg} \n')
    return wrapper


def generator_callback_f08_register(procedure_data: ProcedureData) -> GeneratorOutput:
    generator_callback_f08_register.counter += 1
    yield f'integer, parameter :: SCOREP_MPI_REGION__{procedure_data.canonical_name.upper()} = {generator_callback_f08_register.counter}\n'


generator_callback_f08_register.counter = 0


def generate_f08_group_params() -> GeneratorOutput:
    yield '! pure groups, which can be specified in conf\n'
    for pure_group in scp.groups.scorep_pure_groups:
        yield f'integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_{pure_group.upper()} = ibset(0, {generate_f08_group_params.counter})\n'
        generate_f08_group_params.counter += 1

    yield '! derived groups, which are a combination of existing groups\n'
    for derived_group in scp.groups.scorep_derived_groups:
        yield f'integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_{derived_group[0].upper()}_{derived_group[1].upper()} = ibset(0, {generate_f08_group_params.counter})\n'
        generate_f08_group_params.counter += 1

    yield f'integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_REQUEST = ibset(0, {generate_f08_group_params.counter})\n'
    generate_f08_group_params.counter += 1

    yield "! The Fortran 2008 'iany' intrinsic is not used here, because it is\n"
    yield "! not supported by some compilers based on flang/classic:\n"
    yield "! NVHPC <= 24.11 and ROCm <= 6.2.0 are known examples.\n"
    yield "! '+' works, because all operands have exactly one bit set to one.\n"
    yield '! ALL comprises all pure groups\n'
    yield 'integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_ALL = &\n'
    yield '+&\n'.join([f'SCOREP_MPI_ENABLED_{pure_group.upper()}'
                       for pure_group in scp.groups.scorep_all_groups])
    yield '\n'

    yield '! DEFAULT groups\n'
    yield 'integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_DEFAULT = &\n'
    yield '+&\n'.join([f'SCOREP_MPI_ENABLED_{pure_group.upper()}'
                       for pure_group in scp.groups.scorep_default_enabled_groups])
    yield '\n'


generate_f08_group_params.counter = 0
