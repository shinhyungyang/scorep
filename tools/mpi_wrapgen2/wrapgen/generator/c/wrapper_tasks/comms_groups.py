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
__all__ = ['TaskGroupFree',
           'TaskCommCreate',
           'TaskIntercommCreate',
           'TaskCommCreateBase',
           'TaskCommSetName',
           'TaskCommFree',
           'TaskCommCreateRequest',
           'TaskGroupCreate']

from typing import Set, Type

from wrapgen.data.scorep.collectives import CollectiveEndMode
from wrapgen.language.iso_c import CVariable, CParameter
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.scope import Scope
from wrapgen.generator.c.wrapper_tasks.common import TaskGetRequestId


class TaskCommCreateBase(CTask):
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

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_comm_handle', base_type='SCOREP_InterimCommunicatorHandle')
            ),
            CVariable(parameter=CParameter(name='local_new_comm_handle',
                                           base_type='SCOREP_InterimCommunicatorHandle'),
                      initial_value='SCOREP_INVALID_INTERIM_COMMUNICATOR')
        )

    def generate_initialization(self) -> GeneratorOutput:
        if self.has_comm_parent():
            yield f'{self.ln.local_comm_handle} = SCOREP_MPI_COMM_HANDLE({self.comm_parent()});\n'
        else:
            yield f'{self.ln.local_comm_handle} = SCOREP_INVALID_INTERIM_COMMUNICATOR;\n'

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'SCOREP_MpiCollectiveBegin();\n'

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( *{self.comm_new()} != MPI_COMM_NULL )
{{
    {self.ln.local_new_comm_handle} = scorep_mpi_comm_create(*{self.comm_new()}, {self.comm_parent()});
}}
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.local_new_comm_handle} != SCOREP_INVALID_INTERIM_COMMUNICATOR )
{{
    SCOREP_CommCreate({self.ln.local_new_comm_handle});
}}
'''
        if self.collective_end_mode() == CollectiveEndMode.new_or_self:
            yield f'''\
else
{{
    /* The communicator creation was a local operation, hence we use MPI_COMM_SELF for the collective */
    {self.ln.local_new_comm_handle} = SCOREP_MPI_COMM_HANDLE(MPI_COMM_SELF);
}}
'''
        if self.collective_end_mode() == CollectiveEndMode.parent:
            collective_end_handle = self.ln.local_comm_handle
        else:
            collective_end_handle = self.ln.local_new_comm_handle
        yield f'''\
SCOREP_MpiCollectiveEnd({collective_end_handle},
                        SCOREP_INVALID_ROOT_RANK,
                        SCOREP_COLLECTIVE_CREATE_HANDLE,
                        0,
                        0);
'''


class TaskIntercommCreate(TaskCommCreate):
    def needs_local_variables(self) -> Scope[CVariable]:
        scope = super().needs_local_variables()
        scope_update = Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_rank', base_type='int')
            )
        )
        scope.update(scope_update)
        return scope

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('local_comm', 'local_leader')

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( *{self.comm_new()} != MPI_COMM_NULL )
{{
    /* The semantic of the peer communicator only requires a useful
     * communicator on the respective local leader rank.
     * To avoid unnecessary duplicates use MPI_COMM_NULL for all other
     * ranks in the place of the parent/peer communicator.
     */
    PMPI_Comm_rank( {self.ipn.local_comm}, &{self.ln.local_rank} );
    if ( {self.ipn.local_leader} == {self.ln.local_rank} )
    {{
        {self.ln.local_new_comm_handle} = scorep_mpi_comm_create( *{self.comm_new()}, {self.comm_parent()} );
    }}
    else
    {{
        {self.ln.local_new_comm_handle} = scorep_mpi_comm_create( *{self.comm_new()}, MPI_COMM_NULL );
    }}
}}
'''


class TaskCommCreateRequest(TaskCommCreateBase):
    @staticmethod
    def dependencies() -> Set[Type[CTask]]:
        return {TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'SCOREP_MpiNonBlockingCollectiveRequest( {self.ln.local_request_id} );\n'

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( return_value == MPI_SUCCESS )
{{
     scorep_mpi_request_comm_idup_create( *{self.ipn.request}, {self.comm_parent()}, {self.comm_new()}, {self.ln.local_request_id});
}}
'''


class TaskCommFree(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_comm_handle', base_type='SCOREP_InterimCommunicatorHandle')
            )
        )

    def generate_initialization(self) -> GeneratorOutput:
        yield f'{self.ln.local_comm_handle} = SCOREP_MPI_COMM_HANDLE(*{self.ipn.comm});\n'

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'SCOREP_MpiCollectiveBegin();\n'

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f'scorep_mpi_comm_free(*{self.ipn.comm});\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
SCOREP_CommDestroy( {self.ln.local_comm_handle} );
SCOREP_MpiCollectiveEnd( {self.ln.local_comm_handle},
                         SCOREP_INVALID_ROOT_RANK,
                         SCOREP_COLLECTIVE_DESTROY_HANDLE,
                         0,
                         0);
'''


class TaskGroupCreate(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping(group_new=self.get_attribute('new'))

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( *{self.ipn.group_new} != MPI_GROUP_NULL )
{{
    scorep_mpi_group_create(*{self.ipn.group_new});
}}
'''


class TaskGroupFree(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('group')

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f'scorep_mpi_group_free( *{self.ipn.group} );\n'


class TaskCommSetName(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'comm_name')

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( return_value == MPI_SUCCESS )
{{
    scorep_mpi_comm_set_name( {self.ipn.comm}, {self.ipn.comm_name} );
}}
'''
