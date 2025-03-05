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
__all__ = ['TaskCollectiveCommunication',
           'TaskNonblockingCollectiveCommunication',
           'TaskCollectiveCommunicationByteCalc']

from typing import Set, Type

from wrapgen.data.scorep.collectives import collective_base_name, collective_args, scorep_collective_type
from wrapgen.language.iso_c import CParameter, CVariable
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.c.wrapper_tasks.common import TaskGetRequestId
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskCollectiveCommunication(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskCollectiveCommunicationByteCalc}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def root(self):
        if 'root' in self.scp_procedure.scp_parameters:
            return f'scorep_mpi_get_scorep_mpi_rank({self.scp_procedure.scp_parameters["root"].internal_parameter().name})'
        return 'SCOREP_INVALID_ROOT_RANK'

    def generate_enter_if_group_active(self):
        yield 'SCOREP_MpiCollectiveBegin();\n'

    def generate_exit_if_group_active(self):
        yield f'''\
SCOREP_MpiCollectiveEnd(SCOREP_MPI_COMM_HANDLE({self.ipn.comm}),
                        {self.root()},
                        {scorep_collective_type(self.scp_procedure.canonical_name)},
                        {self.ln.local_send_bytes},
                        {self.ln.local_recv_bytes});
'''


class TaskNonblockingCollectiveCommunication(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskGetRequestId, TaskCollectiveCommunicationByteCalc}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'request')

    def root(self):
        if 'root' in self.scp_procedure.scp_parameters:
            return self.scp_procedure.scp_parameters['root'].internal_parameter().name
        return 'SCOREP_INVALID_ROOT_RANK'

    def generate_enter_if_group_active(self):
        yield f'SCOREP_MpiNonBlockingCollectiveRequest({self.ln.local_request_id});\n'

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ln.return_value} == MPI_SUCCESS )
{{
    scorep_mpi_request_icoll_create(*{self.ipn.request},
                                    SCOREP_MPI_REQUEST_FLAG_NONE,
                                    {scorep_collective_type(self.scp_procedure.canonical_name)},
                                    {self.root()},
                                    {self.ln.local_send_bytes},
                                    {self.ln.local_recv_bytes},
                                    {self.ipn.comm},
                                    {self.ln.local_request_id});
}}
'''


class TaskCollectiveCommunicationByteCalc(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_send_bytes', base_type='uint64_t'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='local_recv_bytes', base_type='uint64_t'),
                initial_value='0'
            )
        )

    def generate_enter_if_group_active(self):
        coll_name = collective_base_name(self.scp_procedure.canonical_name)
        if coll_name not in collective_args:
            raise KeyError(f"Unsupported collective '{coll_name}' in {self.scp_procedure.canonical_name}.")

        args = collective_args[coll_name]['args']
        args = [self.scp_procedure.scp_parameters[a].internal_parameter().name for a in args]
        if collective_args[coll_name]['inplace-arg'] is not None:
            inplace_arg = collective_args[coll_name]['inplace-arg']
            inplace_arg_internal = self.scp_procedure.scp_parameters[inplace_arg].internal_parameter().name
            args.append(f'({inplace_arg_internal} == MPI_IN_PLACE)')
        args.append(self.ipn.comm)
        args.append('&local_send_bytes')
        args.append('&local_recv_bytes')

        args_str = ', '.join(args)

        if self.scp_procedure.is_embiggened():
            coll_name = coll_name + '_c'
        yield f'scorep_mpi_coll_bytes_{coll_name}({args_str});\n'
