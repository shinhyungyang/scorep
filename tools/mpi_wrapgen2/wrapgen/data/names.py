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
import re
from collections import UserDict


def canonical_mpi_name(name: str) -> str:
    """
    The canonical mpi name corresponds to the keys of PROCEDURES in mpistandard
    i.e., all lower-case, with mpi_ prefix, no _c suffix
    """
    return 'mpi_' + bare(name).lower()


def is_canonical(name: str) -> bool:
    return canonical_mpi_name(name) == name


def c_name(name: str) -> str:
    """
    C bindings for MPI functions have this case pattern: MPI_Xxx_yyy_zzz
    """
    return 'MPI_' + bare(name).capitalize()


def remove_prefix(name: str) -> str:
    return re.sub(pattern=r'^P?MPI_', repl='', string=name, flags=re.IGNORECASE)


def remove_postfix(name: str) -> str:
    return re.sub(pattern=r'_C$', repl='', string=name, flags=re.IGNORECASE)


def bare(name: str) -> str:
    return remove_prefix(remove_postfix(name))


class NamespaceDict(UserDict):
    def __init__(self, _key_transform=lambda x: x, _dict=None, /, **kwargs):
        self._key_transform = _key_transform
        super().__init__(_dict, **kwargs)

    def __getitem__(self, key):
        name = self._key_transform(key)
        return self.data[name]

    def __setitem__(self, key, value):
        name = self._key_transform(key)
        self.data[name] = value

    def __delitem__(self, key):
        name = self._key_transform(key)
        del self.data[name]

    def __contains__(self, key):
        name = self._key_transform(key)
        return name in self.data

    @property
    def key_transform(self):
        return self._key_transform


class MPINamespaceDict(NamespaceDict):
    def __init__(self, _dict=None, /, **kwargs):
        super().__init__(canonical_mpi_name, _dict, **kwargs)
