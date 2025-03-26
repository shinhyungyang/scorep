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
import inspect

from wrapgen.language import VariableBase, ParameterBase
from wrapgen.generator.wrapper import Wrapper, ScorepProcedure
from wrapgen.data.mpi_version import Version
from wrapgen.generator.scope import OrderedAttrDict


class ScpProcedureDummy(ScorepProcedure):
    def __init__(self, name: str):
        super(ScpProcedureDummy, self).__init__(canonical_name=name, version=Version('1', '0'))

    @property
    def _get_std_procedure(self):
        raise NotImplementedError()

    @property
    def scp_parameters(self):
        raise NotImplementedError()

    def is_embiggened(self) -> bool:
        raise NotImplementedError()


class DependencyTracker(Wrapper):
    def __init__(self):
        super().__init__()
        self._scp_procedure = ScpProcedureDummy(name='Dependency Tracker')

    @property
    def scp_procedure(self) -> ScorepProcedure:
        return self._scp_procedure

    def local_variables(self) -> dict:
        local_vars = dict()
        for task in self.attached_tasks():
            try:
                my_local_vars = task.needs_local_variables()
            except (AttributeError, NotImplementedError):
                task_name = type(task).__name__
                my_local_vars = OrderedAttrDict.from_named_items(
                    VariableBase(parameter=ParameterBase(name=f'local_from_{task_name}'),
                                 initial_value=f'Local variables from {task_name} are procedure specific and cannot be shown here'))
            local_vars.update(my_local_vars)
        return local_vars

    def get_hooks(self) -> dict[str, list]:
        all_hooks = dict[str, list]()
        for task in self.attached_tasks():
            hooks = inspect.getmembers(type(task),
                                       predicate=lambda m: inspect.isfunction(m) and m.__name__.startswith('generate_'))
            for h in hooks:
                if h[0] not in all_hooks:
                    all_hooks[h[0]] = []
                all_hooks[h[0]].append(type(task))
        return all_hooks

    def generate(self):
        pass


def print_tasks(module_name, task_types):
    print(f'List of tasks in module {module_name}:')
    print('#' * 80)
    print()
    for task_type_name, task_type in sorted(task_types.items(), key=lambda x: x[0]):
        dt = DependencyTracker()
        # Add the non-auxiliary dependencies
        for task_dep in task_type.dependencies():
            if not task_dep.is_auxiliary() and task_dep not in [type(t) for t in dt.attached_tasks()]:
                dt.add_task(task_type=task_dep)
        # Add the task we want to print
        dt.add_task(task_type=task_type)

        print()
        print(f'Task: {task_type_name}')
        print('-' * 80)
        print('\tDependencies: ')
        for dep in dt.attached_tasks()[:-1]:
            automatic_dependency = '(aux)' if dep.is_auxiliary() else ''
            print(f'\t\t{dep.__class__.__name__}{automatic_dependency}')

        print('\tLocal variables: ')
        for local in dt.local_variables().values():
            print(f"\t\t{local}")

        print('\tHooks: ')
        for hook_name, hook_from in dt.get_hooks().items():
            print(f'\t\t{hook_name}(from {", ".join([h.__name__ for h in hook_from])})')

        print('-' * 80)
