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
from typing import Iterable, Union, Dict, Callable

from wrapgen.data import PROCEDURES, EXCLUDED_PROCEDURES, ProcedureSets, get_predefined_procedure_set, ProcedureData

from .wrapper_hooks import GeneratorOutput
from .wrapper_status import WrapperStatus
from .debug_output import enable_debug_output, disable_debug_output, is_debug_enabled
from . import f, c, f08, autoconf
from . import utils_c, utils_f
from . import print_tasks

from enum import Enum
import logging

from .. import errors

_logger = logging.getLogger(__name__)


def init(enable_debug=False):
    if enable_debug:
        enable_debug_output()


class Layer(Enum):
    C = 'C',
    F = 'F',
    F08 = 'F08',
    AUTOCONF = 'Autoconf'


GENERATED_WRAPPERS: Dict[Layer, Dict[str, WrapperStatus]] = {
    Layer.C: c.GENERATED_WRAPPERS,
    Layer.F: f.GENERATED_WRAPPERS,
    Layer.F08: f08.GENERATED_WRAPPERS,
}


def generate_for_procedures(procedures: Union[ProcedureSets, Iterable[str]],
                            callback: Callable[[ProcedureData], GeneratorOutput],
                            sort: bool = False) -> GeneratorOutput:
    if isinstance(procedures, ProcedureSets):
        procedure_names = get_predefined_procedure_set(procedures)
    else:
        procedure_names = procedures
    if sort:
        procedure_names = sorted(procedure_names)
    for proc in procedure_names:
        yield from generate_for_procedure(proc, callback)


def string_from_generator_output(generator_output: GeneratorOutput) -> str:
    return ''.join([item for item in generator_output])


def generate_for_procedure(procedure_name: str,
                           callback: Callable[[ProcedureData], GeneratorOutput]) -> GeneratorOutput:
    _logger.debug("Generating procedure '%s' with callback '%s'", procedure_name, callback.__name__)
    if procedure_name not in PROCEDURES:
        if procedure_name in EXCLUDED_PROCEDURES:
            errors.error_if_strict(
                f"Requested to generate from '{callback.__name__}' for '{procedure_name}', "
                f"but there is no entry for that name in scorep.json",
                _logger)
        else:
            errors.error_if_strict(
                f"Requested to generate from '{callback.__name__}' for '{procedure_name}', "
                f"but there is no procedure with this name in the MPI standard.",
                _logger)
        return
    procedure_data = PROCEDURES[procedure_name]
    yield from callback(procedure_data)


def print_tasks_in_layer(layer: Layer):
    """
    This function provides introspection into the task system.

    Print all tasks, their dependencies, local variables, and hooks for a given
    layer.

    :param layer: Print task info for this layer. Supported are C and F08.
    """
    if layer is Layer.C:
        print_tasks.print_tasks(c.__name__, c.get_public_task_types())
    elif layer is Layer.F08:
        print_tasks.print_tasks(f08.__name__, f08.get_public_task_types())
    else:
        raise NotImplementedError(f"This function is only implemented for layers C and F08")
