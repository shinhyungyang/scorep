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
__all__ = ['TaskCollectiveCommunicationByteCalc',
           'TaskNonblockingCollectiveCommunication',
           'TaskCollectiveCommunication']

from typing import Dict, Set, Type

from wrapgen.data.scorep.collectives import collective_base_name, collective_args, scorep_collective_type
from wrapgen.language.fortran import FortranArgument, FortranVariable
from wrapgen.generator.f08 import scorep_f08_macros
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.f08.wrapper_tasks.common import TaskGetRequestId
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskCollectiveCommunicationByteCalc(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_send_bytes',
                    type_name='integer',
                    type_subtype='c_int64_t'),
                initial_value='0_c_int64_t'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_recv_bytes',
                    type_name='integer',
                    type_subtype='c_int64_t'),
                initial_value='0_c_int64_t'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_in_place',
                    type_name='logical',
                    type_subtype='c_bool'),
                initial_value='.false.'
            )
        )

    def generate_enter_if_group_active(self):
        coll_name = collective_base_name(self.scp_procedure.canonical_name)
        if coll_name not in collective_args:
            raise KeyError(f"Unsupported collective '{coll_name}' in {self.scp_procedure.canonical_name}.")

        args = collective_args[coll_name]['args']
        args = [self.scp_procedure.scp_parameters[a].internal_parameter().name for a in args]
        if collective_args[coll_name]['inplace-arg'] is not None:
            inplace_arg = self.scp_procedure.scp_parameters[
                collective_args[coll_name]['inplace-arg']].internal_parameter().name
            yield f'''\
#if {scorep_f08_macros.HAVE(self.scp_procedure.choice_buffers_macro())}
    {self.ln.local_in_place} = scorep_mpi_is_in_place_ts({inplace_arg})
#else
    {self.ln.local_in_place} = scorep_mpi_is_in_place({inplace_arg})
#endif
'''
            args.append(self.ln.local_in_place)
        args.append(self.ipn.comm)
        args.append(self.ln.local_send_bytes)
        args.append(self.ln.local_recv_bytes)
        args_str = ', '.join(args)
        yield f'call scorep_mpi_coll_bytes_{coll_name}({args_str})\n'


class TaskCollectiveCommunication(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskCollectiveCommunicationByteCalc}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def root(self):
        if 'root' in self.scp_procedure.scp_parameters:
            return f'scorep_mpi_get_scorep_mpi_rank({self.scp_procedure.scp_parameters["root"].internal_parameter().name})'
        return 'SCOREP_INVALID_ROOT_RANK'

    def generate_enter_if_group_active(self):
        yield 'call SCOREP_MpiCollectiveBegin()\n'

    def generate_exit_if_group_active(self):
        yield f'''\
call SCOREP_MpiCollectiveEnd(scorep_mpi_comm_handle({self.ipn.comm}), &
                             {self.root()}, &
                             {scorep_collective_type(self.scp_procedure.canonical_name)}, &
                             {self.ln.local_send_bytes}, &
                             {self.ln.local_recv_bytes})
'''


class TaskNonblockingCollectiveCommunication(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetRequestId, TaskCollectiveCommunicationByteCalc}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'request', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def root(self):
        if 'root' in self.scp_procedure.scp_parameters:
            return self.scp_procedure.scp_parameters['root'].internal_parameter().name
        return 'SCOREP_INVALID_ROOT_RANK'

    def generate_enter_if_group_active(self):
        yield f'call SCOREP_MpiNonBlockingCollectiveRequest({self.ln.local_request_id})\n'

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call scorep_mpi_request_icoll_create({self.ipn.request}, &
                                         SCOREP_MPI_REQUEST_FLAG_NONE, &
                                         {scorep_collective_type(self.scp_procedure.canonical_name)}, &
                                         {self.root()}, &
                                         {self.ln.local_send_bytes}, &
                                         {self.ln.local_recv_bytes}, &
                                         {self.ipn.comm}, &
                                         {self.ln.local_request_id})
end if
'''
