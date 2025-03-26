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
__all__ = ['TaskCommCreate',
           'TaskIntercommCreate',
           'TaskGroupCreate',
           'TaskCommSetName',
           'TaskCommFree',
           'TaskCommCreateRequest',
           'TaskGroupFree',
           'TaskCommCreateBase']

from typing import Dict, Set, Type

from wrapgen.data.scorep.collectives import CollectiveEndMode
from wrapgen.language.fortran import FortranArgument, FortranVariable
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.scope import Scope
from wrapgen.generator.f08.wrapper_tasks.common import TaskGetRequestId


class TaskCommCreateBase(F08Task):
    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_COMM_NULL'}

    def comm_new(self) -> str:
        return self.get_attribute('new', lambda name: self.scp_procedure.scp_parameters[name].internal_parameter().name)

    def has_comm_parent(self) -> bool:
        return self.attributes.get('parent', None) is not None

    def comm_parent(self) -> str:
        return self.get_attribute('parent', lambda name: self.scp_procedure.scp_parameters[
            name].internal_parameter().name if name is not None else 'MPI_COMM_NULL')


class TaskCommCreate(TaskCommCreateBase):
    def collective_end_mode(self) -> CollectiveEndMode:
        return self.get_attribute('collective_end_mode', CollectiveEndMode)

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_comm_handle',
                    type_name='integer',
                    type_subtype='SCOREP_InterimCommunicatorHandle')
            ),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_new_comm_handle',
                    type_name='integer',
                    type_subtype='SCOREP_InterimCommunicatorHandle'
                ),
                initial_value='SCOREP_INVALID_INTERIM_COMMUNICATOR'
            )
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        uses = super().needs_mpi_f08_uses()
        uses.update({'MPI_COMM_SELF'})
        return uses

    def generate_initialization(self) -> GeneratorOutput:
        if self.has_comm_parent():
            yield f'{self.ln.local_comm_handle} = scorep_mpi_comm_handle({self.comm_parent()})\n'
        else:
            yield f'{self.ln.local_comm_handle} = SCOREP_INVALID_INTERIM_COMMUNICATOR\n'

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_MpiCollectiveBegin()\n'

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ({self.comm_new()} .ne. MPI_COMM_NULL) then
    {self.ln.local_new_comm_handle} = scorep_mpi_comm_create({self.comm_new()}, {self.comm_parent()})
end if
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.local_new_comm_handle} .ne. SCOREP_INVALID_INTERIM_COMMUNICATOR ) then
    call SCOREP_CommCreate({self.ln.local_new_comm_handle})
'''
        if self.collective_end_mode() == CollectiveEndMode.new_or_self:
            yield f'''\
else
    ! The communicator creation was a local operation, hence we use MPI_COMM_SELF for the collective
    {self.ln.local_new_comm_handle} = scorep_mpi_comm_handle(MPI_COMM_SELF)
'''
        yield 'end if\n'
        if self.collective_end_mode() == CollectiveEndMode.parent:
            collective_end_handle = self.ln.local_comm_handle
        else:
            collective_end_handle = self.ln.local_new_comm_handle
        yield f'''\
call SCOREP_MpiCollectiveEnd({collective_end_handle}, &
                             SCOREP_INVALID_ROOT_RANK, &
                             SCOREP_COLLECTIVE_CREATE_HANDLE, &
                             0_c_int64_t, &
                             0_c_int64_t)
'''


class TaskIntercommCreate(TaskCommCreate):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        scope = super().needs_local_variables()
        scope_update = Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(name='local_rank', type_name='integer')
            )
        )
        scope.update(scope_update)
        return scope

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('local_comm', 'local_leader')

    def needs_mpi_f08_uses(self) -> set[str]:
        uses = super().needs_mpi_f08_uses()
        uses.update({'PMPI_Comm_rank'})
        return uses

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ({self.comm_new()} .ne. MPI_COMM_NULL) then
    ! The semantic of the peer communicator only requires a useful
    ! communicator on the respective local leader rank.
    ! To avoid unnecessary duplicates use MPI_COMM_NULL for all other
    ! ranks in the place of the parent/peer communicator.

    call PMPI_Comm_rank( {self.ipn.local_comm}, {self.ln.local_rank} )
    if ( {self.ipn.local_leader} .eq. {self.ln.local_rank} ) then
        {self.ln.local_new_comm_handle} = scorep_mpi_comm_create( {self.comm_new()}, {self.comm_parent()} )
    else
        {self.ln.local_new_comm_handle} = scorep_mpi_comm_create( {self.comm_new()}, MPI_COMM_NULL )
    end if
end if
'''


class TaskCommCreateRequest(TaskCommCreateBase):
    @staticmethod
    def dependencies() -> Set[Type[F08Task]]:
        return {TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        uses = super().needs_mpi_f08_uses()
        uses.update({'MPI_SUCCESS'})
        return uses

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'call SCOREP_MpiNonBlockingCollectiveRequest( {self.ln.local_request_id} )\n'

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.ierror} .ne. MPI_SUCCESS ) then
     call scorep_mpi_request_comm_idup_create( {self.ipn.request}, {self.comm_parent()}, {self.comm_new()}, {self.ln.local_request_id})
end if
'''


class TaskCommFree(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(parameter=FortranArgument.new(
                name='local_comm_handle',
                type_name='integer',
                type_subtype='SCOREP_InterimCommunicatorHandle'
            ))
        )

    def generate_initialization(self) -> GeneratorOutput:
        yield f'{self.ln.local_comm_handle} = scorep_mpi_comm_handle({self.ipn.comm})\n'

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_MpiCollectiveBegin()\n'

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f'call scorep_mpi_comm_free({self.ipn.comm})\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_CommDestroy({self.ln.local_comm_handle})
call SCOREP_MpiCollectiveEnd({self.ln.local_comm_handle}, &
                             SCOREP_INVALID_ROOT_RANK, &
                             SCOREP_COLLECTIVE_DESTROY_HANDLE, &
                             0_c_int64_t, &
                             0_c_int64_t)
'''


class TaskGroupCreate(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping(group_new=self.get_attribute('new'))

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_GROUP_NULL'}

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ({self.ipn.group_new} .ne. MPI_GROUP_NULL) then
    call scorep_mpi_group_create({self.ipn.group_new})
end if
'''


class TaskGroupFree(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('group')

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f'call scorep_mpi_group_free({self.ipn.group})\n'


class TaskCommSetName(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'comm_name', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call scorep_mpi_comm_set_name( {self.ipn.comm}, {self.ipn.comm_name} )
end if
'''
