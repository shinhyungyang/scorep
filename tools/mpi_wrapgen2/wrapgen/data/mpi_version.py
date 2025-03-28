# This file is part of the Score-P software (http://www.score-p.org)
#
# Copyright (c) 2025,
# Forschungszentrum Juelich GmbH, Germany
#
# This software may be modified and distributed under the terms of
# a BSD-style license. See the COPYING file in the package base
# directory for details.
#
from typing import Tuple, Iterable, Optional, Callable, Union


class Version:
    def __init__(self, *args: Union[str, int, Iterable]) -> None:
        """
        Construct a Version object from either
        - a string '<major>' or '<major>.<minor>'
        - a tuple of integers (<major>,) or (<major>, <minor>)
        - a tuple of strings ('<major>',) or ('<major>', '<minor>')
        """
        if len(args) == 1:
            if isinstance(args[0], str):
                tokens = args[0].split('.')
            elif isinstance(args[0], Iterable):
                tokens = args[0]
            else:
                raise TypeError(f'Need either a string or a tuple, got {type(args[0])}.')
        else:
            tokens = args
        assert len(tokens) <= 2, "A version has at most 2 components: major and optionally minor version number"
        major = int(tokens[0])
        assert major >= 0, f"Major version number {major} is negative"
        if len(tokens) >= 2:
            minor = int(tokens[1])
            assert minor >= 0, f"Minor version number {minor} is negative"
            self._version_tuple: Tuple[int, ...] = (major, minor)
        else:
            self._version_tuple: Tuple[int, ...] = (major,)

    @property
    def major(self) -> int:
        return self._version_tuple[0]

    def has_minor(self) -> bool:
        return len(self._version_tuple) >= 2

    @property
    def minor(self) -> Optional[int]:
        if self.has_minor():
            return self._version_tuple[1]
        else:
            return None

    def tuple(self) -> Tuple[int, ...]:
        return self._version_tuple

    def __str__(self) -> str:
        return '.'.join([str(v) for v in self._version_tuple])

    def __repr__(self) -> str:
        return self.__str__()

    def __hash__(self):
        return self._version_tuple.__hash__()

    def _compare(self,
                 rhs,
                 cmp: Callable[[Tuple[int, ...], Tuple[int, ...]], bool]) -> Union[bool, type(NotImplemented)]:
        if isinstance(rhs, Version):
            # use lexicographic ordering of tuples:
            return cmp(self.tuple(), rhs.tuple())
        return NotImplemented

    def __le__(self, other):
        return self._compare(other, lambda s, o: s <= o)

    def __ge__(self, other):
        return self._compare(other, lambda s, o: s >= o)

    def __eq__(self, other):
        return self._compare(other, lambda s, o: s == o)

    def __ne__(self, other):
        return self._compare(other, lambda s, o: s != o)

    def __lt__(self, other):
        return self._compare(other, lambda s, o: s < o)

    def __gt__(self, other):
        return self._compare(other, lambda s, o: s > o)


mpi_standard_versions = (
    Version(1, 0),
    Version(2, 0),
    Version(2, 1),
    Version(2, 2),
    Version(3, 0),
    Version(3, 1),
    Version(4, 0)
)


def is_valid_mpi_version(version: Version):
    return version in mpi_standard_versions
