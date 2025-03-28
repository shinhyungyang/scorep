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
from typing import List
from collections import OrderedDict

from wrapgen.data import mpistandard as std
from wrapgen.data.names import remove_postfix
from wrapgen.data.mpi_version import Version
from wrapgen.generator.f08.scorep_f08_parameter import ScorepF08Parameter
from wrapgen.language.fortran import FortranArgument
from wrapgen.generator.scorep_procedure import ScorepProcedure


class ScorepF08Procedure(ScorepProcedure):
    def __init__(self, std_procedure: std.F08Symbol, canonical_name: str, version: Version, **kwargs):
        super().__init__(canonical_name, version, **kwargs)
        self._std_procedure = std_procedure

        self._scp_parameters = OrderedDict[str, ScorepF08Parameter]()
        for p in self._std_procedure.parameters:
            self._scp_parameters[p.name] = ScorepF08Parameter(std_parameter=p,
                                                              **self._parameter_extra_info.get(p.name, {}))

    def _get_std_procedure(self) -> std.F08Symbol:
        return self._std_procedure

    @property
    def scp_parameters(self) -> OrderedDict[str, ScorepF08Parameter]:
        return self._scp_parameters

    def dummy_parameters(self) -> List[FortranArgument]:
        return [p.decl_param() for p in self.scp_parameters.values()]

    def is_embiggened(self) -> bool:
        return isinstance(self._get_std_procedure(), std.EmbiggenedF08Procedure)

    def symbol_macro(self) -> str:
        name = f'SCOREP_F08_SYMBOL_NAME_{remove_postfix(self._get_std_procedure().name).upper()}'
        if self.is_embiggened():
            name = name + '_C'
        return name

    def choice_buffers_macro(self) -> str:
        name = f'F08_TS_BUFFERS_{remove_postfix(self._get_std_procedure().name).upper()}'
        if self.is_embiggened():
            name = name + '_C'
        return name
