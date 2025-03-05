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
Represent Fortran and C language entities(types, function arguments, ...) as classes.
Extract language information from pympistandard classes.
"""

from dataclasses import dataclass


@dataclass
class ParameterBase:
    name: str = ''


@dataclass
class VariableBase:
    parameter: ParameterBase
    initial_value: str = ''

    @property
    def name(self):
        return self.parameter.name

    def __str__(self):
        return f'VAR {self.name} := {self.initial_value}'
