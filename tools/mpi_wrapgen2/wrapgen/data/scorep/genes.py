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
from typing import Any, Dict
from dataclasses import dataclass, field

from wrapgen.data.mpi_version import Version
from wrapgen.data.scorep.regions import RegionType


@dataclass(init=True, frozen=True)
class TaskData:
    typename: str
    attributes: dict = field(default_factory=dict)


@dataclass(init=True, frozen=True)
class Genes:
    """
    Frozen dataclass holding all Score-P related information of a single MPI
    procedure.
    """
    mpi_version: Version = field(default_factory=lambda: Version(0, 0))
    """Version of the MPI Standard in which the procedure was introduced."""
    large_mpi_version: Version = field(default_factory=lambda: Version(0, 0))
    """Version of the MPI Standard in which the embiggened procedure was introduced."""
    group: str = 'MISC'
    """Score-P function group."""
    guards: list = field(default_factory=list)
    """Additional guards to remove the wrapper at compile-time."""
    region: RegionType = RegionType.none
    """Score-P region type"""
    parameters: dict = field(default_factory=dict)
    """Per-parameter information specific to this procedure. Keys are parameter
    names according to the MPI standard. Possible values are defined by
    :class:`ScorepParameter`."""
    template: str = 'DefaultWrapper'
    """Name of the class used to generate this procedure."""
    c_variants: list = field(default_factory=list)
    """Alternative, non-standard-compliant declarations to check for.
    Use in combination with template=ProtoArgsWrapper.
    Only relevant for C-bindings."""
    tasks: list[TaskData] = field(default_factory=list)
    """
    Additional functionality for the wrapper.
    """

    @staticmethod
    def from_raw(data: Dict[str, Any]) -> 'Genes':
        if 'mpi_version' in data:
            data['mpi_version'] = Version(data['mpi_version'])
            assert data['mpi_version'].has_minor()
        if 'large_mpi_version' in data:
            data['large_mpi_version'] = Version(data['large_mpi_version'])
            assert data['large_mpi_version'].has_minor()
        if 'region' in data:
            data['region'] = RegionType(data['region'])
        if 'tasks' in data:
            for idx in range(len(data['tasks'])):
                data['tasks'][idx] = TaskData(**data['tasks'][idx])
        return Genes(**data)

    def get_task(self, name: str):
        for t in self.tasks:
            if t.typename == name:
                return t
        return None
