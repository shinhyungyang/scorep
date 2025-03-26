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
import abc
from collections import OrderedDict
from typing import List, Union

from wrapgen.generator.scorep_parameter import ScorepParameter
from wrapgen.data import mpistandard as std
from wrapgen.data.mpi_version import Version
from wrapgen.language import ParameterBase


class ScorepProcedure(abc.ABC):
    def __init__(self, canonical_name: str, version: Version, **kwargs):
        self._canonical_name = canonical_name
        self._version = version
        self._parameter_extra_info = kwargs

    @property
    def canonical_name(self) -> str:
        return self._canonical_name

    @property
    def std_name(self) -> str:
        return self._get_std_procedure().name

    @abc.abstractmethod
    def is_embiggened(self) -> bool:
        pass

    @property
    def version(self) -> Version:
        # The embiggened procedures were introduced in 4.0
        if self.is_embiggened() and self._version < Version(4, 0):
            return Version(4, 0)
        return self._version

    @abc.abstractmethod
    def _get_std_procedure(self) -> Union[std.ISOCSymbol, std.F08Symbol]:
        pass

    @property
    @abc.abstractmethod
    def scp_parameters(self) -> OrderedDict[str, ScorepParameter]:
        pass

    def internal_parameters(self) -> List[ParameterBase]:
        return [p.internal_parameter()
                for p in self.scp_parameters.values()
                if p.has_internal_replacement()]
