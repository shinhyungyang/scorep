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
__all__ = ['TaskSend',
           'TaskRecv',
           'TaskIrecv',
           'TaskIsend',
           'TaskSendInit',
           'TaskRecvInit',
           'TaskCommHandle',
           'TaskGetSendBytes',
           'TaskGetRecvBytes',
           'TaskGetRecvBytesFromStatus']

from typing import Set, Type

from wrapgen.language.iso_c import CParameter, CVariable
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope
from wrapgen.generator.c.wrapper_tasks.common import TaskGetRequestId


class TaskSend(CTask):
    @staticmethod
    def dependencies() -> Set[Type[CTask]]:
        return {TaskGetSendBytes, TaskCommHandle}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest', 'comm', sendtag=['sendtag', 'tag'])

    def generate_enter_if_group_active(self):
        yield f'''\
if ( {self.ipn.dest} != MPI_PROC_NULL )
{{
    SCOREP_MpiSend( {self.ipn.dest}, {self.ln.local_comm_handle}, {self.ipn.sendtag}, {self.ln.local_send_bytes} );
}}
'''


class TaskRecv(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskGetRecvBytesFromStatus, TaskCommHandle}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'comm', 'status')

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.source} != MPI_PROC_NULL && return_value == MPI_SUCCESS )
{{
    SCOREP_MpiRecv( {self.ipn.status}->MPI_SOURCE, {self.ln.local_comm_handle}, {self.ipn.status}->MPI_TAG, {self.ln.local_recv_bytes} );
}}
'''


class TaskIsend(CTask):
    @staticmethod
    def dependencies() -> Set[Type[CTask]]:
        return {TaskGetSendBytes, TaskCommHandle, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest', 'comm', 'request',
                                                sendtag=['sendtag', 'tag'],
                                                sendtype=['sendtype', 'datatype'])

    def generate_enter_if_group_active(self):
        yield f'''\
if ( dest != MPI_PROC_NULL )
{{
    SCOREP_MpiIsend( {self.ipn.dest}, {self.ln.local_comm_handle},
                     {self.ipn.sendtag}, {self.ln.local_send_bytes}, {self.ln.local_request_id} );
}}
'''

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.dest} != MPI_PROC_NULL && {self.ln.return_value} == MPI_SUCCESS )
{{
    scorep_mpi_request_p2p_create( *{self.ipn.request},
                                   SCOREP_MPI_REQUEST_TYPE_SEND,
                                   SCOREP_MPI_REQUEST_FLAG_NONE,
                                   {self.ipn.sendtag},
                                   {self.ipn.dest},
                                   {self.ln.local_send_bytes},
                                   {self.ipn.sendtype},
                                   {self.ipn.comm},
                                   {self.ln.local_request_id} );
}}
'''


class TaskIrecv(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskGetRecvBytes, TaskCommHandle, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'comm', 'request', 'tag', recvtype=['recvtype', 'datatype'])

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.source} != MPI_PROC_NULL && return_value == MPI_SUCCESS )
{{
    SCOREP_MpiIrecvRequest( local_request_id );
    scorep_mpi_request_p2p_create(*{self.ipn.request},
                                  SCOREP_MPI_REQUEST_TYPE_RECV,
                                  SCOREP_MPI_REQUEST_FLAG_NONE,
                                  {self.ipn.tag},
                                  0,
                                  {self.ln.local_recv_bytes},
                                  {self.ipn.recvtype},
                                  {self.ipn.comm},
                                  {self.ln.local_request_id});
}}
'''


class TaskSendInit(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskGetSendBytes, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest', 'comm', 'request', 'tag', 'datatype')

    def generate_exit_if_group_active(self):
        yield f'''\
if ({self.ipn.dest} != MPI_PROC_NULL && {self.ln.return_value} == MPI_SUCCESS )
{{
    scorep_mpi_request_p2p_create( *{self.ipn.request},
                                   SCOREP_MPI_REQUEST_TYPE_SEND,
                                   SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT,
                                   {self.ipn.tag},
                                   {self.ipn.dest},
                                   {self.ln.local_send_bytes},
                                   {self.ipn.datatype},
                                   {self.ipn.comm},
                                   {self.ln.local_request_id} );
}}
'''


class TaskRecvInit(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskGetRecvBytes, TaskGetRequestId}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'comm', 'request', 'tag', 'datatype')

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.source} != MPI_PROC_NULL && {self.ln.return_value} == MPI_SUCCESS )
{{
    scorep_mpi_request_p2p_create(*{self.ipn.request},
                                  SCOREP_MPI_REQUEST_TYPE_RECV,
                                  SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT,
                                  {self.ipn.tag},
                                  {self.ipn.source},
                                  {self.ln.local_recv_bytes},
                                  {self.ipn.datatype},
                                  {self.ipn.comm},
                                  {self.ln.local_request_id});
}}
'''


class TaskCommHandle(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(
                    name='local_comm_handle',
                    base_type='SCOREP_InterimCommunicatorHandle',
                    const=True),
                initial_value=f'SCOREP_MPI_COMM_HANDLE({self.ipn.comm})'
            )
        )


class TaskGetSendBytes(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('dest',
                                                sendtype=['sendtype', 'datatype'],
                                                sendcount=['sendcount', 'count'])

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_send_bytes', base_type='uint64_t'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='local_send_type_size', base_type='int'),
                initial_value='0'
            )
        )

    def generate_enter_if_group_active(self):
        yield f'''\
if ({self.ipn.dest} != MPI_PROC_NULL)
{{
    PMPI_Type_size( {self.ipn.sendtype}, &{self.ln.local_send_type_size} );
    {self.ln.local_send_bytes} = (uint64_t) {self.ipn.sendcount} * {self.ln.local_send_type_size};
}}
'''


class TaskGetRecvBytesFromStatus(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source', 'status', recvtype=['recvtype', 'datatype'])

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_recv_bytes', base_type='uint64_t'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='local_recv_type_size', base_type='int'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='local_recv_count', base_type='int'),
                initial_value='0'
            )
        )

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.source} != MPI_PROC_NULL && return_value == MPI_SUCCESS )
{{
    PMPI_Type_size({self.ipn.recvtype}, &{self.ln.local_recv_type_size});
    PMPI_Get_count({self.ipn.status}, {self.ipn.recvtype}, &{self.ln.local_recv_count});
    {self.ln.local_recv_bytes} = (uint64_t) {self.ln.local_recv_count} * {self.ln.local_recv_type_size};
}}
'''


class TaskGetRecvBytes(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('source',
                                                recvcount=['recvcount', 'count'],
                                                recvtype=['recvtype', 'datatype'])

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_recv_bytes', base_type='uint64_t'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='local_recv_type_size', base_type='int'),
                initial_value='0'
            )
        )

    def generate_exit_if_group_active(self):
        yield f'''\
if ( {self.ipn.source} != MPI_PROC_NULL && return_value == MPI_SUCCESS )
{{
    PMPI_Type_size({self.ipn.recvtype}, &{self.ln.local_recv_type_size});
    {self.ln.local_recv_bytes} = (uint64_t) {self.ipn.recvcount} * {self.ln.local_recv_type_size};
}}
'''
