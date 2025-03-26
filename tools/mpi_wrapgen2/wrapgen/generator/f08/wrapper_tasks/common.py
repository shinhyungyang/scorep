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
__all__ = ['TaskDefault', 'TaskTodo', 'TaskFatal', 'TaskGetRequestId']

from typing import Type, Dict

from wrapgen.language.fortran import FortranVariable, FortranArgument
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.f08 import scorep_f08_macros
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskDefault(F08Task):
    """
    To mark that the default behavior is explicitly required.

    The generator assumes that wrappers with no attached tasks are not fully
    implemented and marks them as 'todo'.
    Adding this task suppresses that 'todo'.
    """

    @staticmethod
    def has_conflict_with(task_type: Type['F08Task']) -> bool:
        """
        :class:`TaskDefault` should be the only task, thus it has conflicts with
        all other tasks.
        """
        return True

    def generate_comment(self) -> GeneratorOutput:
        yield '!> Default wrapper: Only enter and exit events are created.\n'


class TaskTodo(F08Task):
    """
    To mark that the currently implemented behavior is not enough.
    """

    @staticmethod
    def allow_multiple_instances() -> bool:
        """
        Allow to add more than one task of type :class:`TaskTodo` to the wrapper.

        :return: True
        """
        return True

    def generate_comment(self) -> GeneratorOutput:
        msg = self.get_attribute('message', str, 'This wrapper is not fully implemented.')
        yield f'!> TODO: {msg} \n'


class TaskFatal(F08Task):
    """
    Wrappers with this task abort the program with an error message.
    """

    def generate_comment(self) -> GeneratorOutput:
        yield '!> Aborts the measurement.\n'

    def generate_initialization(self) -> GeneratorOutput:
        msg = self.get_attribute('message', str)
        yield scorep_f08_macros.UTILS_FATAL(msg)


class TaskGetRequestId(AuxiliaryTaskMixin, F08Task):
    """
    Add a local variable to the wrapper, which is set to a new Score-P request id.
    """

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(parameter=FortranArgument.new(
                name='local_request_id',
                type_name='integer',
                type_subtype='SCOREP_MpiRequestId'
            ))
        )

    def generate_enter_if_group_active(self):
        yield f'{self.ln.local_request_id} = scorep_mpi_get_request_id()\n'
