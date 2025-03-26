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
from typing import Iterator, List, Any, Callable, Optional

GeneratorOutput = Iterator[str]
GeneratorFunction = Callable[[Any], GeneratorOutput]


class WrapperHooks:
    """
    Base class that provides the mechanism to attach tasks and execute the
    tasks' hooks.
    """

    def __init__(self) -> None:
        self._attached_tasks: List[Any] = []
        self._debug_formatter: Callable[[Any], str] = lambda x: ''

    def attach(self, task: Any) -> None:
        """
        Attach a task.
        """
        self._attached_tasks.append(task)

    def attached_tasks(self) -> List[Any]:
        """
        :return: Read-only access to the currently attached tasks.
        """
        return self._attached_tasks[:]

    def clear_attached_tasks(self) -> None:
        """
        Remove all attached tasks.
        """
        self._attached_tasks = []

    def set_debug_formatter(self, callback: Callable[[str], str]) -> None:
        self._debug_formatter = callback

    def unset_debug_formatter(self):
        self._debug_formatter = lambda x: ''

    def _generate_debug(self, msg: str) -> GeneratorOutput:
        formatted_msg = self._debug_formatter(msg)
        if formatted_msg != '':
            yield formatted_msg

    @staticmethod
    def hook(post: Optional[GeneratorFunction] = None) -> Callable[[GeneratorFunction], GeneratorFunction]:
        """
        Decorator for member functions.

        The original function must be a string wrapgen without arguments.

        The result of the decoration:

        1. yields the output from the original function,
        2. for all attached tasks, checks whether the task has a member with the
           same name and if so, yields the output from that function,
        3. yields from post if present.

        :param post: A member function of the class
        :return: Function decorator
        """

        def fun_decorator(fun: GeneratorFunction) -> GeneratorFunction:
            """
            This is the transformation from the member function fun to the decorated function.

            :param fun: Member function to be decorated
            :return: The decorated function visible to the caller of 'fun'
            """

            def decorated_fun(self) -> GeneratorOutput:
                """
                This is the result of decoration that will be visible to the caller of the original 'fun'.

                :param self: The object that 'fun' is a member of.
                :return: Iterator over generated output.
                """
                yield from self._generate_debug(f"Hook '{fun.__name__}' enter")
                yield from fun(self)
                for obj in self._attached_tasks:
                    if hasattr(obj, fun.__name__):
                        callback: Callable = obj.__getattribute__(fun.__name__)
                        yield from self._generate_debug(f"'{type(obj).__name__}' enter hook")
                        yield from callback()
                        yield from self._generate_debug(f"'{type(obj).__name__}': exit hook")
                    else:
                        yield from self._generate_debug(f"'{type(obj).__name__}': hook not registered")
                        yield from ()
                if post is not None:
                    yield from self._generate_debug(f"Hook '{fun.__name__}' post")
                    yield from post(self)
                yield from self._generate_debug(f"Hook '{fun.__name__}' exit")

            df = decorated_fun
            df.__doc__ = f"""Hook for tasks

            {fun.__doc__ if fun.__doc__ else ""}
            """
            return df

        return fun_decorator
