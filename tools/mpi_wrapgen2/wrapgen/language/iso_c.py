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
"""
Classes to represent C function parameters and variables.
"""

from dataclasses import dataclass, field
from typing import List

from wrapgen.language import ParameterBase, VariableBase


def parse_array(array_string: str) -> List[str]:
    """
    From '[a][b][]' get the list ['a', 'b', '']
    """
    if array_string is None or len(array_string) == 0:
        return []
    # As long as we don't need to parse anything else there is no point in writing a real parser.
    # Assume that output from mpistandard does not contain syntax errors.
    array_string = array_string.replace('][', ',').lstrip('[').rstrip(']')
    return array_string.split(',')


@dataclass
class CParameter(ParameterBase):
    base_type: str = ''
    pointer_level: int = 0
    arrays: list = ()
    const: bool = False

    def is_pointer(self) -> bool:
        return self.pointer_level > 0 or len(self.arrays) > 0

    def decl(self) -> str:
        const_str = 'const ' if self.const else ''
        return const_str \
            + self.base_type \
            + '*' * self.pointer_level \
            + ' ' + self.name \
            + ''.join([f'[{a}]' for a in self.arrays])

    def type(self) -> str:
        return self.base_type + "*" * (self.pointer_level + len(self.arrays))

    def __str__(self) -> str:
        return self.decl()


@dataclass
class CVariable(VariableBase):
    parameter: CParameter = field(default_factory=CParameter)

    def __str__(self):
        assert not (self.parameter.const and self.initial_value == ''), 'Constant variables must be initialized'
        init_str = f' = {self.initial_value}' if self.initial_value != '' else ''
        return f'{self.parameter}{init_str};'
