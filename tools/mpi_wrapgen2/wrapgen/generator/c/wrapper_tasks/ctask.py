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
__all__ = ['CTask']

from typing import Optional, Dict, Set, Type, cast, Any

from wrapgen.language.iso_c import CVariable, CParameter
from wrapgen.generator.wrapper import Wrapper
from wrapgen.generator.c.scorep_c_procedure import ScorepCProcedure
from wrapgen.generator.scope import Scope
from wrapgen.generator.task import ProcedureTask
from wrapgen.generator.print_tasks import DependencyTracker


class CTask(ProcedureTask):
    """
    Base class for all Tasks in the C bindings.
    """

    def __init__(self, parent: Wrapper, scp_procedure: ScorepCProcedure, attributes: Optional[Dict] = None):
        self._parent = parent
        self._scp_procedure = scp_procedure
        if attributes is not None:
            self._attributes = attributes
        else:
            self._attributes = dict()
        # Order is important:
        # The scopes may use information in self._attributes
        # The creation of the local scope with 'needs_local_variables' may depend on the internal scope
        if not isinstance(self._parent, DependencyTracker):
            self._internal_scope = Scope[CParameter](((k, self.scp_procedure.scp_parameters[v].internal_parameter())
                                                      for k, v in self.needs_parameter_names().items()))
            self._local_scope = cast(Scope[CVariable], self.parent.all_local_variables())
            # At creation, this task is not yet attached to self.parent
            self._local_scope.update(self.needs_local_variables())

    @property
    def parent(self) -> Wrapper:
        return self._parent

    @property
    def scp_procedure(self) -> ScorepCProcedure:
        return self._scp_procedure

    @property
    def ipsc(self) -> Scope[CParameter]:
        return self._internal_scope

    @property
    def lsc(self) -> Scope[CVariable]:
        return self._local_scope

    @property
    def attributes(self) -> Dict[str, Any]:
        return self._attributes

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        """
        Override in subclasses to add task dependencies.

        See :meth:`ProcedureTask.dependencies()`.

        :return: The default provided by this method is an empty set of dependencies.
        """
        return set()

    def needs_parameter_names(self) -> Scope[str]:
        """
        Override in subclasses to specify the names of MPI procedure parameters
        that the task uses.

        See :meth:`ProcedureTask.needs_parameter_names()`.

        :return: The default provided by this method is an empty :class:`Scope` of str.
        """
        return Scope()

    def needs_local_variables(self) -> Scope[CVariable]:
        """
        Override in subclasses to specify the local variables declared by the
        task.

        See :meth:`ProcedureTask.needs_local_variables()`.

        :return: The default provided by this method is an empty :class:`Scope` of
                 :class:`CVariable`.
        """
        return Scope()
