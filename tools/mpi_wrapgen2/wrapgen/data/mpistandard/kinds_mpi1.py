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
The old MPI1 standard defined some interfaces with incompatible types
between C(MPI_Aint) and Fortran(INTEGER).
These interfaces have been deprecated since MPI2.0 and removed in MPI3.0.

They are not included in the 4.0 apis.json file and no corresponding kinds
are defined in pympistandard.

Note: For functions that have parameters of one of these kinds,
we cannot (safely) implement wrappers in C for the Fortran bindings
(as is done for the f90 bindings).
"""
from pympistandard.kind import Kind

DISPLACEMENT_MPI1 = Kind(
    name="DISPLACEMENT_MPI1",
    _lis="integer",
    _iso_c_small="MPI_Aint",
    _f90_small="INTEGER",
    _f08_small="INTEGER",
)

DTYPE_STRIDE_BYTES_MPI1 = Kind(
    name="DTYPE_STRIDE_BYTES_MPI1",
    _lis="integer",
    _iso_c_small="MPI_Aint",
    _f90_small="INTEGER",
    _f08_small="INTEGER",
)
