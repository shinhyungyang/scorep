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
from collections import OrderedDict

from wrapgen.data import mpistandard as std
from wrapgen.generator.c.scorep_c_parameter import ScorepCParameter
from wrapgen.generator.scorep_procedure import ScorepProcedure
from wrapgen.data.mpi_version import Version


class ScorepCProcedure(ScorepProcedure):
    def __init__(self, std_procedure: std.ISOCSymbol, canonical_name: str, version: Version, **kwargs):
        super().__init__(canonical_name, version, **kwargs)
        self._std_procedure = std_procedure
        self._scp_parameters = OrderedDict[str, ScorepCParameter]()
        for p in self._std_procedure.parameters:
            self._scp_parameters[p.name] = ScorepCParameter(std_parameter=p,
                                                            **self._parameter_extra_info.get(p.name, {}))

    def _get_std_procedure(self) -> std.ISOCSymbol:
        return self._std_procedure

    @property
    def scp_parameters(self) -> OrderedDict[str, ScorepCParameter]:
        return self._scp_parameters

    def is_embiggened(self) -> bool:
        return isinstance(self._get_std_procedure(), std.EmbiggenedISOCSymbol)

    @property
    def return_type(self) -> str:
        return self._get_std_procedure().return_type
