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
from __future__ import annotations
import abc
import inspect
from typing import Type, Dict, Optional, List

from .wrapper_hooks import WrapperHooks, GeneratorOutput
from wrapgen.generator.scorep_procedure import ScorepProcedure
from wrapgen.language import VariableBase
from wrapgen.generator.scope import Scope
from wrapgen.generator.task import ProcedureTask, TaskDependencyError


class Wrapper(WrapperHooks, abc.ABC):
    """
    Generates the wrapper code for an MPI procedure, using info from
    :class:`ScorepProcedure`.

    This abstract base provides a mechanism to add tasks to modify the output.
    See :meth:`add_task`.

    Subclasses for the different bindings define the structure of :meth:`generate`
    and the available hooks:

    - C bindings: :class:`wrapgen.generate.c.c_wrappers.DefaultWrapper`
    - F08 bindings: :class:`wrapgen.generate.f08.f08_wrappers.DefaultWrapper`

    """

    def __init__(self):
        super(Wrapper, self).__init__()

    @abc.abstractmethod
    def generate(self) -> GeneratorOutput:
        """
        Generate the entire procedure wrapper.

        :return: Iterator over the strings that form the wrapper code.
        """
        pass

    @property
    @abc.abstractmethod
    def scp_procedure(self) -> ScorepProcedure:
        """
        :return: Language specific information on the MPI procedure
        """
        pass

    @abc.abstractmethod
    def local_variables(self) -> Scope[VariableBase]:
        """
        Subclasses my override this method to define local variables available in
        the wrapper code.

        :return: Local variables defined in the wrapper
        """
        pass

    def all_local_variables(self) -> Scope[VariableBase]:
        """
        :return: Local variables defined in the wrapper and all attached tasks.
        """
        all_locals = self.local_variables()
        for task in self.attached_tasks():
            all_locals.update(task.needs_local_variables())
        return all_locals

    def add_task(self, task_type: Type[ProcedureTask], attributes: Optional[Dict] = None):
        """
        Add a task.

        Rules for adding tasks:

        - At most one task of any type can be attached to a Wrapper
          object.
        - If add_task is called twice with the same type, an Exception will be
          raised.
        - Tasks that need attributes have to be added explicitly through the
          add_task function.
        - Dependencies that are auxiliary tasks (have the AuxiliaryTaskMixin as
          a base class) are added automatically. Auxiliary tasks are not passed
          any attributes.
        - If two tasks both depend on the same auxiliary task, it will only be
          added once.
        - Dependencies that are not auxiliary cannot be added automatically
          (as they might depend on attributes). If such a dependency is not met,
          an Exception will be raised.
        - If the task to be added (either explicitly or as an automatic
          dependency) conflicts with already attached tasks, an exception will
          be raised.
        """
        self._add_auxiliary_dependencies(task_type)
        self._add_task(task_type, attributes)

    def _add_auxiliary_dependencies(self, task_type: Type[ProcedureTask]):
        """
        Add dependencies that are auxiliary tasks recursively.
        """
        aux_dependencies = sorted([t for t in task_type.dependencies() if t.is_auxiliary()], key=lambda x: x.__name__)
        for dependency in aux_dependencies:
            if dependency not in self._attached_task_types():
                self._add_auxiliary_dependencies(dependency)
                self._add_task(dependency)

    def _add_task(self, task_type: Type[ProcedureTask], attributes=None):
        """
        Construct a tasks of type task_type with attributes and attach it to self.

        Raise Exceptions if :
        - A task with the given task_type is already attached
        - The task_type has a conflict with another already attached task
        - A dependency of the task_type is not met.
        - An auxiliary task has a non-auxiliary dependency.
        """
        if task_type in self._attached_task_types() and not task_type.allow_multiple_instances():
            raise TaskDependencyError(
                f'{self.scp_procedure.canonical_name}: Cannot add the same task twice ({task_type.__name__})')

        for tt in self._attached_task_types():
            if task_type.has_conflict_with(tt):
                raise TaskDependencyError(
                    f'{self.scp_procedure.canonical_name}: Task {task_type.__name__} conflicts with {tt.__name__}')

        for dependency in task_type.dependencies():
            dependency_met = False
            for att in self._attached_task_types():
                bases = inspect.getmro(att)
                if dependency in bases:
                    dependency_met = True
                    break

            if not dependency_met:
                raise TaskDependencyError(
                    f'{self.scp_procedure.canonical_name}: Dependency of task {task_type.__name__} is not fulfilled({dependency.__name__})')

        if task_type.is_auxiliary():
            for dependency in task_type.dependencies():
                if not dependency.is_auxiliary():
                    raise TaskDependencyError(
                        f'Auxiliary task {task_type.__name__} has a non-auxiliary dependency {dependency.__name__}')

        if attributes is None:
            attributes = {}
        task = task_type(self, self.scp_procedure, attributes)
        self.attach(task)

    def _attached_task_types(self) -> List[Type[ProcedureTask]]:
        return [type(t) for t in self.attached_tasks()]
