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
Extract Fortran language information from pympistandard parameters
"""
from typing import Union

from wrapgen.data import mpistandard as std
from wrapgen.language.fortran import FortranArgumentDescriptor, FortranArgument


def make_fortran_argument_descriptor(
        parameter: Union[std.F90Parameter, std.F08ParameterBase]) -> FortranArgumentDescriptor:
    if isinstance(parameter, std.F90Parameter):
        descriptor = FortranArgumentDescriptor.from_string(parameter.type, parameter.__str__())
    elif isinstance(parameter, std.F08ParameterBase):
        descriptor = FortranArgumentDescriptor.from_string(parameter.descriptor, parameter.__str__())
    else:
        raise TypeError(f'Wrong argument type {type(parameter)}')
    return descriptor


def make_fortran_argument(parameter: Union[std.F90Parameter, std.F08ParameterBase]) -> FortranArgument:
    return FortranArgument(name=parameter.name,
                           descriptor=make_fortran_argument_descriptor(parameter))
