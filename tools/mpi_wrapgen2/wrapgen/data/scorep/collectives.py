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
from enum import Enum
from typing import Dict, Any
import re

from .. import names


class CollectiveEndMode(Enum):
    parent = 'parent'
    new_or_self = 'new_or_self'


def collective_base_name(procedure_name) -> str:
    """
    Strip MPI_ prefix, _c suffix and _init suffix.
    Strip I prefix.

    Example:
    MPI_Ibcast_init_c -> bcast
    MPI_Neighbor_alltoall -> neighbor_alltoall
    """
    base_name = names.bare(procedure_name).lower()
    base_name = re.sub(pattern=r'_init$', repl='', string=base_name)
    base_name = re.sub(pattern=r'^i', repl='', string=base_name)
    return base_name


def scorep_collective_type(procedure_name) -> str:
    base_name = collective_base_name(procedure_name)
    if base_name == 'bcast':
        base_name = 'broadcast'
    base_name = re.sub(pattern=r'^neighbor_', repl='', string=base_name)
    return f"SCOREP_COLLECTIVE_{base_name.upper()}"


collective_args: Dict[str, Dict[str, Any]] = {
    'barrier': {'args': (),
                'inplace-arg': None},
    'gather': {'args': ('sendcount', 'sendtype', 'recvcount', 'recvtype', 'root'),
               'inplace-arg': 'sendbuf'},
    'reduce': {'args': ('count', 'datatype', 'root'),
               'inplace-arg': 'sendbuf'},
    'gatherv': {'args': ('sendcount', 'sendtype', 'recvcounts', 'recvtype', 'root'),
                'inplace-arg': 'sendbuf'},
    'bcast': {'args': ('count', 'datatype', 'root'),
              'inplace-arg': None},
    'scatter': {'args': ('sendcount', 'sendtype', 'recvcount', 'recvtype', 'root'),
                'inplace-arg': 'recvbuf'},
    'scatterv': {'args': ('sendcounts', 'sendtype', 'recvcount', 'recvtype', 'root'),
                 'inplace-arg': 'recvbuf'},
    'alltoall': {'args': ('sendcount', 'sendtype', 'recvcount', 'recvtype'),
                 'inplace-arg': 'sendbuf'},
    'alltoallv': {'args': ('sendcounts', 'sendtype', 'recvcounts', 'recvtype'),
                  'inplace-arg': 'sendbuf'},
    'alltoallw': {'args': ('sendcounts', 'sendtypes', 'recvcounts', 'recvtypes'),
                  'inplace-arg': 'sendbuf'},
    'allgather': {'args': ('sendcount', 'sendtype', 'recvcount', 'recvtype'),
                  'inplace-arg': 'sendbuf'},
    'allgatherv': {'args': ('sendcount', 'sendtype', 'recvcounts', 'recvtype'),
                   'inplace-arg': 'sendbuf'},
    'allreduce': {'args': ('count', 'datatype'),
                  'inplace-arg': 'sendbuf'},
    'reduce_scatter_block': {'args': ('recvcount', 'datatype'),
                             'inplace-arg': 'sendbuf'},
    'reduce_scatter': {'args': ('recvcounts', 'datatype'),
                       'inplace-arg': 'sendbuf'},
    'scan': {'args': ('count', 'datatype'),
             'inplace-arg': 'sendbuf'},
    'exscan': {'args': ('count', 'datatype'),
               'inplace-arg': 'sendbuf'},
    'neighbor_alltoall': {'args': ('sendcount', 'sendtype', 'recvcount', 'recvtype'),
                          'inplace-arg': None},
    'neighbor_alltoallv': {'args': ('sendcounts', 'sendtype', 'recvcounts', 'recvtype'),
                           'inplace-arg': None},
    'neighbor_alltoallw': {'args': ('sendcounts', 'sendtypes', 'recvcounts', 'recvtypes'),
                           'inplace-arg': None},
    'neighbor_allgather': {'args': ('sendcount', 'sendtype', 'recvcount', 'recvtype'),
                           'inplace-arg': None},
    'neighbor_allgatherv': {'args': ('sendcount', 'sendtype', 'recvcounts', 'recvtype'),
                            'inplace-arg': None},
}
