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
Extract C language information from pympistandard parameters
"""
from wrapgen.data import mpistandard as std
from wrapgen.language.iso_c import CParameter, parse_array


def make_c_parameter(parameter: std.ISOCParameter) -> CParameter:
    # Pympistandard 0.1 returns the wrong base_type in EmbiggenedISOCParameter
    # Work around by using the type property and stripping off pointers
    if isinstance(parameter, std.EmbiggenedISOCParameter):
        base_type = parameter.type.rstrip('*').rstrip()
    else:
        base_type = parameter.base_type

    return CParameter(name=parameter.name,
                      base_type=base_type,
                      pointer_level=parameter.pointer_level,
                      arrays=parse_array(parameter.array),
                      const=parameter.constant)
