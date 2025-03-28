"""
A Task encapsulates one piece of additional behavior of a wrapper.

More than one task can be added to a wrapper. The generated wrapper will contain
code from all tasks that are 'attached' to the :class:`wrapgen.Wrapper` class.
"""
from __future__ import annotations

import abc
from typing import Optional, Dict, Set, Type, Any

from wrapgen.generator.scorep_procedure import ScorepProcedure
from wrapgen.generator.scope import Scope
from wrapgen.language import VariableBase, ParameterBase


class ProcedureTask(abc.ABC):
    """
    Tasks are 'attached' to a :class:`Wrapper` to generate additional
    output.

    This abstract base class defines, amongst other things, the interface for
    access to the 'parent' :class:`Wrapper`, the :class:`ScorepProcedure`
    data, variable scopes and the dependencies/conflicts with other tasks.

    Tasks for the C bindings should be derived from :class:`CTask`, and
    tasks for the F08 bindings from :class:`F08Task`.
    """

    @abc.abstractmethod
    def __init__(self, parent: 'Wrapper',
                 scp_procedure: ScorepProcedure,
                 attributes: Optional[Dict] = None) -> None:
        """
        The constructor initializes in that order:

        1. The internal parameter scope, calling :meth:`needs_parameter_names` to
           assign names to the MPI procedure's parameters, and using data from
           scp_procedure.
        2. The local scope, merging the local scope of the parent with the result
           of :meth:`needs_local_variables`.

        :param parent: Where the task is attached.
        :param scp_procedure: Data on the MPI procedure. Should be ``parent.scp_procedure``
        :param attributes: Keys/values are defined by the concrete task implementations.
        """
        pass

    @property
    @abc.abstractmethod
    def parent(self) -> 'Wrapper':
        """
        :return: The wrapper this task is attached to.
        """
        pass

    @property
    @abc.abstractmethod
    def scp_procedure(self) -> ScorepProcedure:
        """
        Convenience access, same as parent.scp_procedure

        :return: Data on the MPI procedure.
        """
        pass

    @abc.abstractmethod
    def needs_local_variables(self) -> Scope[VariableBase]:
        """
        Subclasses can override this method to define local variables
        for a task.

        This method is called during a task's :meth:`__init__`, after initialization of
        attributes, and after :meth:`needs_parameter_names`.
        That is, local variables can make use of attributes and the
        parameter information.

        :return: A mapping from local variable names to local variable data.
        """
        pass

    @abc.abstractmethod
    def needs_parameter_names(self) -> Scope[str]:
        """
        Subclasses must override this method if the task depends on the parameter
        names of the MPI procedure.

        The overriding implementation should construct the mapping via a call
        to :meth:`make_parameter_name_mapping`.

        This method is called during a task's :meth:`__init__`, after initialization of
        attributes but before :meth:`needs_local_variables`.
        That is, the mapping can depend on the task attributes.

        :return: A mapping from names used in the task to names of MPI procedure
                 parameters.
        """
        pass

    @staticmethod
    def dependencies() -> Set[Type['ProcedureTask']]:
        """
        Subclasses can override this method to define the dependencies of a task.
        Dependencies are defined per class, not per instance of a task.

        ``TaskA in TaskB.dependencies()`` means that TaskA must be attached to
        the parent prior to TaskB. If TaskA is an auxiliary task, it is attached
        automatically.

        See :meth:`Wrapper.add_task()`.

        :return: The set of task types this task depends on.
        """
        return set()

    @staticmethod
    def has_conflict_with(task_type: Type['ProcedureTask']) -> bool:
        """
        Subclasses can override this method to define the conflicts with other
        task types.
        Conflicts are defined per class, not per instance.

        ``TaskB.has_conflict_with(TaskA) == True`` means that TaskA must not
        be attached to the same parent prior to TaskB.

        :param task_type: Query conflict with this task type.
        :return: Whether this task type has a conflict with task_type.
        """
        return False

    @staticmethod
    def allow_multiple_instances() -> bool:
        """
        Subclasses can override this method to allow multiple instances of a
        task to be attached to the same parent.

        This does not make sense for most tasks. An exception is for example
        :class:`TaskTodo`, which only generates a comment.

        :return: False, if not overridden.
        """
        return False

    @staticmethod
    def is_auxiliary() -> bool:
        """
        :return: Whether the task is auxiliary. To mark a task as auxiliary, derive
                 additionally from the :class:`AuxiliaryTaskMixin`.
        """
        return False

    @property
    @abc.abstractmethod
    def ipsc(self) -> Scope[ParameterBase]:
        """
        The 'Internal Parameter SCope': An ordered dictionary of
        aliases for MPI procedure parameter names and associated info.

        If only the internal name is needed, use :meth:`ipn` instead.

        :return: Read access to the internal scope.
        """
        pass

    @property
    @abc.abstractmethod
    def lsc(self) -> Scope[VariableBase]:
        """
        The 'Local SCope': An ordered dictionary of the local variables defined
        by the task.

        If only the name of a local variable is needed, use :meth:`ln` instead.

        :return: Read access to the local scope.
        """
        pass

    @property
    def ipn(self) -> Scope[str]:
        """
        The 'Internal Parameter Names': A mapping from MPI procedure parameters
        to wrapper-internal names.

        :return: Read access to the internal parameter names.
        """
        return Scope((k, v.name) for k, v in self.ipsc.items())

    @property
    def ln(self) -> Scope[str]:
        """
        The 'Local Parameter Names': names for local variables defined by the
        task.

        :return: Read access to the local names.
        """
        return Scope((k, v.name) for k, v in self.lsc.items())

    def make_parameter_name_mapping(self, /, *args, **kwargs) -> Scope[str]:
        """
        This helper method should be used in subclasses' calls to :meth:`needs_parameter_names`
        to construct the scope of internal parameter names.

        The items in 'args' are taken as parameter names. For each item, the corresponding MPI procedure
        is checked, whether it takes a parameter with that name.

        With 'kwargs' one can define aliases for one or more alternative parameter
        names. For example ``make_parameter_name_mapping(status=['status', 'recvstatus'])``
        defines 'status' as an alias name for either 'status' or 'recvstatus'
        if the MPI procedure takes exactly one parameter named 'status' or 'recvstatus'.

        :raise KeyError: If one of the given names/aliases is not a parameter of the corresponding MPI procedure or if
                         more than one alias matches a parameter name.

        :param args: Names of MPI procedure parameters to be included in the scope.
        :param kwargs: Keyword: Alias for a parameter name, Value: either a single parameter name or a list of alternative names.
        :return: A mapping from aliases for parameter names to the actual parameter names.
        """
        name_mapping = Scope()
        for name in args:
            if name not in self.scp_procedure.scp_parameters:
                raise KeyError(f"No parameter named '{name}' in '{self.scp_procedure.std_name}'")
            name_mapping[name] = name
        for name, alt in kwargs.items():
            if isinstance(alt, str):
                if alt not in self.scp_procedure.scp_parameters:
                    raise KeyError(f"No parameter named '{alt}' in '{self.scp_procedure.std_name}'")
                name_mapping[name] = alt
            elif isinstance(alt, list):
                matching_keys = tuple(filter(lambda key: key in alt, self.scp_procedure.scp_parameters.keys()))
                if len(matching_keys) == 0:
                    raise KeyError(f"None of {alt} are parameters of '{self.scp_procedure.std_name}'")
                if len(matching_keys) > 1:
                    raise KeyError(f"More than one of {alt} are parameters of '{self.scp_procedure.std_name}")
                name_mapping[name] = matching_keys[0]
        return name_mapping

    @property
    @abc.abstractmethod
    def attributes(self) -> Dict[str, Any]:
        """
        :return: Access to the task attributes. Subclasses can define freely
                 which keys/values are allowed or required.
        """
        pass

    def get_attribute(self, attribute_name, attribute_type=None, default=None):
        """
        Access an attribute by name and convert it to the given type.

        :param attribute_name: Attribute name: Key in the 'attributes' dict.
        :param attribute_type: Call this method on the value if the key is present in the 'attributes' dict.
                               Can be a constructor or a lambda.
        :param default: If the key is not present, return the default instead.

        :raise ValueError: if the key is not present in 'attributes' and no
                   default is given.

        :return: The attribute converted to the type.
        """
        if attribute_name in self.attributes:
            raw_attribute = self.attributes[attribute_name]
            if attribute_type is None:
                return raw_attribute
            try:
                attribute = attribute_type(raw_attribute)
                return attribute
            except:
                raise ValueError(f"Attribute '{attribute_name}' of task {self.__class__} attached to "
                                 f"'{self.scp_procedure.std_name}': Could not convert '{raw_attribute}' to "
                                 f"'{attribute_type}'")
        else:
            if default is not None:
                return default
            else:
                raise KeyError(f"Task {self.__class__} attached to '{self.scp_procedure.std_name}': "
                               f"Attribute '{attribute_name}' is missing and does not have a default")


class AuxiliaryTaskMixin:
    """
    Derive from this mixin in addition to :class::`ProcedureTask`  to mark a
    tasks as auxiliary.

    Auxiliary tasks cannot depend on attributes, but they are automatically
    attached, if another task depends on them.
    """

    @staticmethod
    def is_auxiliary() -> bool:
        return True


class TaskDependencyError(Exception):
    pass
