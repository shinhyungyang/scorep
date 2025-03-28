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

from typing import Type

from wrapgen.generator import GeneratorOutput
from wrapgen.language.iso_c import CParameter, CVariable
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskDefault(CTask):
    """
    To mark that the default behavior is explicitly required.

    The generator assumes that wrappers with no attached tasks are not fully
    implemented and marks them as 'todo'.
    Adding this task suppresses that 'todo'.
    """

    @staticmethod
    def has_conflict_with(task_type: Type['CTask']) -> bool:
        """
        :class:`TaskDefault` should be the only task, thus it has conflicts with
        all other tasks.
        """
        return True

    def generate_comment(self) -> GeneratorOutput:
        yield '* Default wrapper: Only enter and exit events are created.\n'


class TaskTodo(CTask):
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
        yield f'* TODO: {msg}\n'


class TaskFatal(CTask):
    """
    Wrappers with this task abort the program with an error message.
    """

    def generate_comment(self) -> GeneratorOutput:
        yield '* Aborts the measurement.\n'

    def generate_initialization(self) -> GeneratorOutput:
        msg = self.get_attribute('message', str)
        yield f'UTILS_FATAL( "{msg}" );\n'


class TaskGetRequestId(AuxiliaryTaskMixin, CTask):
    """
    Add a local variable to the wrapper, which is set to a new Score-P request id.
    """

    def needs_local_variables(self) -> Scope[CParameter]:
        """
        Adds a local variable of type SCOREP_MpiRequestId.
        """
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_request_id', base_type='SCOREP_MpiRequestId')
            )
        )

    def generate_enter_if_group_active(self):
        yield f'{self.ln.local_request_id} = scorep_mpi_get_request_id();\n'
