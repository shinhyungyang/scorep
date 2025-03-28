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
__all__ = ['TaskIrecv',
           'TaskRecv',
           'TaskSend',
           'TaskIsend',
           'TaskCommHandle',
           'TaskGetRecvBytes',
           'TaskGetSendBytes',
           'TaskGetRecvBytesFromStatus',
           'TaskRecvInit',
           'TaskSendInit']

from typing import Dict, Set, Type

from wrapgen.language.fortran import FortranArgument, FortranVariable
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope
from wrapgen.generator.f08.wrapper_tasks.common import TaskGetRequestId


class TaskGetSendBytes(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest',
                                                sendtype=['sendtype', 'datatype'],
                                                sendcount=['sendcount', 'count'])

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(parameter=FortranArgument.new(
                name='local_send_bytes',
                type_name='integer',
                type_subtype='c_int64_t'
            )),
            FortranVariable(parameter=FortranArgument.new(
                name='local_send_type_size',
                type_name='integer'
            ))
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'PMPI_Type_size'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.dest} .ne. MPI_PROC_NULL ) then
    call pmpi_type_size({self.ipn.sendtype}, {self.ln.local_send_type_size})
    {self.ln.local_send_bytes} = {self.ipn.sendcount} * {self.ln.local_send_type_size}
endif
'''


class TaskSend(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetSendBytes, TaskCommHandle}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest', 'comm', sendtag=['sendtag', 'tag'])

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.dest} .ne. MPI_PROC_NULL ) then
    call SCOREP_MpiSend({self.ipn.dest}, {self.ln.local_comm_handle}, {self.ipn.sendtag}, {self.ln.local_send_bytes})
endif
'''


class TaskGetRecvBytesFromStatus(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'ierror', 'status', recvtype=['recvtype', 'datatype'])

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(parameter=FortranArgument.new(
                name='local_recv_bytes',
                type_name='integer',
                type_subtype='c_int64_t'
            )),
            FortranVariable(parameter=FortranArgument.new(
                name='local_recv_type_size',
                type_name='integer'
            )),
            FortranVariable(parameter=FortranArgument.new(
                name='local_recv_count',
                type_name='integer'
            ))
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS', 'PMPI_Type_size', 'PMPI_Get_count'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.source} .ne. MPI_PROC_NULL .and. {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call pmpi_type_size({self.ipn.recvtype}, {self.ln.local_recv_type_size})
    call pmpi_get_count({self.ipn.status}, {self.ipn.recvtype}, {self.ln.local_recv_count})
    {self.ln.local_recv_bytes} = {self.ln.local_recv_count} * {self.ln.local_recv_type_size}
end if
'''


class TaskRecv(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetRecvBytesFromStatus, TaskCommHandle}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'comm', 'status', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS', 'MPI_SOURCE', 'MPI_TAG'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.source} .ne. MPI_PROC_NULL .and. {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call SCOREP_MpiRecv({self.ipn.status}%MPI_SOURCE, {self.ln.local_comm_handle}, {self.ipn.status}%MPI_TAG, {self.ln.local_recv_bytes})
end if
'''


class TaskIsend(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetSendBytes, TaskCommHandle, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest', 'comm', 'request', 'ierror',
                                                sendtag=['sendtag', 'tag'],
                                                sendtype=['sendtype', 'datatype'])

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS'}

    def generate_enter_if_group_active(self):
        yield f'''\
if ( {self.ipn.dest} .ne. MPI_PROC_NULL ) then
    call SCOREP_MpiIsend( {self.ipn.dest}, {self.ln.local_comm_handle}, {self.ipn.sendtag}, {self.ln.local_send_bytes}, {self.ln.local_request_id} )
end if
'''

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.dest} .ne. MPI_PROC_NULL .and.  {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call scorep_mpi_request_p2p_create( {self.ipn.request},&
                                        SCOREP_MPI_REQUEST_TYPE_SEND,&
                                        SCOREP_MPI_REQUEST_FLAG_NONE,&
                                        {self.ipn.sendtag},&
                                        {self.ipn.dest},&
                                        {self.ln.local_send_bytes},&
                                        {self.ipn.sendtype},&
                                        {self.ipn.comm},&
                                        {self.ln.local_request_id} )
end if
'''


class TaskCommHandle(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_comm_handle',
                    type_name='integer',
                    type_subtype='SCOREP_InterimCommunicatorHandle'),
                initial_value=f'scorep_mpi_comm_handle({self.ipn.comm})'))


class TaskGetRecvBytes(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'ierror',
                                                recvcount=['recvcount', 'count'],
                                                recvtype=['recvtype', 'datatype'])

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(parameter=FortranArgument.new(
                name='local_recv_bytes',
                type_name='integer',
                type_subtype='c_int64_t'
            )),
            FortranVariable(parameter=FortranArgument.new(
                name='local_recv_type_size',
                type_name='integer'
            ))
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS', 'PMPI_Type_size'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.source} .ne. MPI_PROC_NULL .and. {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call pmpi_type_size({self.ipn.recvtype}, {self.ln.local_recv_type_size})
    {self.ln.local_recv_bytes} = {self.ipn.recvcount} * {self.ln.local_recv_type_size}
end if
'''


class TaskIrecv(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetRecvBytes, TaskCommHandle, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'comm', 'request', 'tag', 'ierror',
                                                recvtype=['recvtype', 'datatype'])

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS'}

    def generate_exit_if_group_active(self):
        yield f'''
if ( {self.ipn.source} .ne. MPI_PROC_NULL .and. {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    {self.ln.local_comm_handle} = scorep_mpi_comm_handle( {self.ipn.comm} )
    call SCOREP_MpiIrecvRequest( local_request_id )
    call scorep_mpi_request_p2p_create( {self.ipn.request}, &
                                        SCOREP_MPI_REQUEST_TYPE_RECV, &
                                        SCOREP_MPI_REQUEST_FLAG_NONE, &
                                        {self.ipn.tag}, &
                                        0, &
                                        {self.ln.local_recv_bytes}, &
                                        {self.ipn.recvtype}, &
                                        {self.ipn.comm}, &
                                        {self.ln.local_request_id} )
end if
'''


class TaskSendInit(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetSendBytes, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest', 'comm', 'request', 'tag', 'datatype', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS'}

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.dest} .ne. MPI_PROC_NULL .and. {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call scorep_mpi_request_p2p_create( {self.ipn.request}, &
                                        SCOREP_MPI_REQUEST_TYPE_SEND, &
                                        SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT, &
                                        {self.ipn.tag}, &
                                        {self.ipn.dest}, &
                                        {self.ln.local_send_bytes}, &
                                        {self.ipn.datatype}, &
                                        {self.ipn.comm}, &
                                        {self.ln.local_request_id} )
end if
'''


class TaskRecvInit(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetRecvBytes, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'comm', 'request', 'tag', 'datatype', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_SUCCESS'}

    def generate_exit_if_group_active(self):
        yield f'''\
if ({self.ipn.source} .ne. MPI_PROC_NULL .and. {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call scorep_mpi_request_p2p_create( &
        {self.ipn.request}, &
        SCOREP_MPI_REQUEST_TYPE_RECV, &
        SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT, &
        {self.ipn.tag}, &
        {self.ipn.source}, &
        {self.ln.local_recv_bytes}, &
        {self.ipn.datatype}, &
        {self.ipn.comm}, &
        {self.ln.local_request_id} )
end if
'''
