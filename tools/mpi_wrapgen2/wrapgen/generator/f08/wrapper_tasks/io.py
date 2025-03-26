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
__all__ = ['TaskFileOpen',
           'TaskFileClose',
           'TaskFileSeek',
           'TaskFileDelete',
           'TaskGetIoHandle',
           'TaskGetDatatypeSize',
           'TaskIoBegin',
           'TaskIoBlocking',
           'TaskIoNonBlocking',
           'TaskIoSplitBegin',
           'TaskIoSplitEnd']

from typing import Set, Type

from wrapgen.data.scorep.io import scorep_operation_mode, OperationMode, scorep_collective_operation_flag, \
    scorep_blocking_operation_flag
from wrapgen.language.fortran import FortranVariable, FortranArgument
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.f08.wrapper_tasks.f08task import F08Task
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope
from wrapgen.generator.f08.wrapper_tasks.common import TaskGetRequestId
from wrapgen.generator.f08.scorep_f08_macros import HAVE


class TaskFileOpen(F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_comm_handle',
                    type_name='integer',
                    type_subtype='SCOREP_InterimCommunicatorHandle')),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_unify_key',
                    type_name='integer',
                    type_subtype='c_int32_t'),
                initial_value='0_c_int32_t'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_file',
                    type_name='integer',
                    type_subtype='SCOREP_IoFileHandle')),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_io_handle',
                    type_name='integer',
                    type_subtype='SCOREP_IoHandleHandle'))
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'filename', 'amode', 'fh', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
{self.ln.local_comm_handle} = scorep_mpi_comm_handle({self.ipn.comm})
{self.ln.local_unify_key} = scorep_mpi_io_get_unify_key( {self.ln.local_comm_handle} )
call SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_MPI, &
                                        SCOREP_IO_HANDLE_FLAG_NONE, &
                                        {self.ln.local_comm_handle}, &
                                        "" )
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.ierror} .eq. MPI_SUCCESS ) then
        {self.ln.local_file} = SCOREP_IoMgmt_GetIoFileHandle( {self.ipn.filename} )
        {self.ln.local_io_handle} = SCOREP_IoMgmt_CompleteHandleCreation(SCOREP_IO_PARADIGM_MPI, {self.ln.local_file}, {self.ln.local_unify_key}, {self.ipn.fh} )
        if( {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
            call SCOREP_IoCreateHandle( {self.ln.local_io_handle}, &
                                        scorep_mpi_io_get_access_mode( {self.ipn.amode} ), &
                                        scorep_mpi_io_get_creation_flags( {self.ipn.amode} ), &
                                        scorep_mpi_io_get_status_flags( {self.ipn.amode} ) )
        end if
    else
        call SCOREP_IoMgmt_DropIncompleteHandle()
    end if
'''


class TaskFileClose(F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_amode',
                    type_name='integer')),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_io_handle',
                    type_name='integer',
                    type_subtype='SCOREP_IoHandleHandle'),
                initial_value='SCOREP_INVALID_IO_HANDLE'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_file',
                    type_name='integer',
                    type_subtype='SCOREP_IoFileHandle'))
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('fh', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS', 'MPI_MODE_DELETE_ON_CLOSE', 'PMPI_File_get_amode'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
{self.ln.local_io_handle} = SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_MPI, {self.ipn.fh} )
call PMPI_File_get_amode( {self.ipn.fh}, {self.ln.local_amode} )
call SCOREP_IoMgmt_PushHandle( {self.ln.local_io_handle} )
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} )
    if({self.ipn.ierror} .ne. MPI_SUCCESS) then
        if( iand({self.ln.local_amode}, MPI_MODE_DELETE_ON_CLOSE) .gt. 0 ) then
            {self.ln.local_file} = SCOREP_IoHandleHandle_GetIoFile( {self.ln.local_io_handle} )
            call SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_MPI, {self.ln.local_file} )
        end if
        call SCOREP_IoDestroyHandle( {self.ln.local_io_handle} )
        call SCOREP_IoMgmt_DestroyHandle( {self.ln.local_io_handle} )
    else
        call SCOREP_IoMgmt_ReinsertHandle( SCOREP_IO_PARADIGM_MPI, {self.ln.local_io_handle} )
    end if
end if
'''


class TaskFileSeek(F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_io_handle',
                    type_name='integer',
                    type_subtype='SCOREP_IoHandleHandle'),
                initial_value='SCOREP_INVALID_IO_HANDLE'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_pos',
                    type_name='integer',
                    type_subtype='MPI_OFFSET_KIND'))
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('fh', 'offset', 'whence')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'PMPI_File_get_position'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_io_handle} = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_MPI, {self.ipn.fh} )\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call PMPI_File_get_position( {self.ipn.fh}, {self.ln.local_pos} )

    call SCOREP_IoSeek( {self.ln.local_io_handle}, &
                        {self.ipn.offset}, &
                        scorep_mpi_io_get_seek_option( {self.ipn.whence} ), &
                        {self.ln.local_pos} )

    call SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} )
end if
'''


class TaskFileDelete(F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_file',
                    type_name='integer',
                    type_subtype='SCOREP_IoFileHandle'),
                initial_value='SCOREP_INVALID_IO_FILE')
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('filename')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_file} = SCOREP_IoMgmt_GetIoFileHandle({self.ipn.filename})\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.local_file} .ne. SCOREP_INVALID_IO_FILE ) then
    call SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_MPI, {self.ln.local_file} )
end if
'''


class TaskGetIoHandle(AuxiliaryTaskMixin, F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_io_handle',
                    type_name='integer',
                    type_subtype='SCOREP_IoHandleHandle'),
                initial_value='SCOREP_INVALID_IO_HANDLE')
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('fh')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_io_handle} = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_MPI, {self.ipn.fh} )\n'


class TaskGetDatatypeSize(AuxiliaryTaskMixin, F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_datatype_size',
                    type_name='integer',
                    type_subtype='c_int64_t'),
                initial_value='0_c_int64_t')
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('datatype')

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_datatype_size} = {self.get_datatype_size_fun()}({self.ipn.datatype})\n'


class TaskIoBegin(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetRequestId, TaskGetDatatypeSize, TaskGetIoHandle}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def collective_flag(self) -> str:
        return scorep_collective_operation_flag(self.get_attribute('collective', bool))

    def blocking_flag(self) -> str:
        return scorep_blocking_operation_flag(self.get_attribute('blocking', bool))

    def operation_offset(self) -> str:
        if 'offset' in self.scp_procedure.scp_parameters:
            return self.scp_procedure.scp_parameters['offset'].internal_parameter().name
        else:
            return 'SCOREP_IO_UNKNOWN_OFFSET'

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call SCOREP_IoOperationBegin( {self.ln.local_io_handle}, &
                                  {self.operation_mode()}, &
                                  iand({self.collective_flag()},{self.blocking_flag()}), &
                                  {self.ipn.count} * {self.ln.local_datatype_size}, &
                                  {self.ln.local_request_id}, &
                                  {self.operation_offset()} )
end if
'''


class TaskIoBlocking(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskIoBegin}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('status', 'datatype')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_count',
                    type_name='integer',
                    type_subtype='MPI_COUNT_KIND' if self.scp_procedure.is_embiggened() else None),
                initial_value='0')
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'PMPI_Get_count'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call PMPI_Get_count( {self.ipn.status}, {self.ipn.datatype}, {self.ln.local_count} )
    call SCOREP_IoOperationComplete( {self.ln.local_io_handle}, &
                                     {self.operation_mode()}, &
                                     {self.ln.local_count} * {self.ln.local_datatype_size}, &
                                     {self.ln.local_request_id} )
end if
call SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} )
'''


class TaskIoNonBlocking(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskIoBegin}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request', 'datatype', 'count', 'fh', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.ierror} == MPI_SUCCESS .and. {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call SCOREP_IoOperationIssued( {self.ln.local_io_handle}, {self.ln.local_request_id} )
    call scorep_mpi_request_io_create( {self.ipn.request}, &
                                       {self.operation_mode()}, &
                                       {self.ipn.count} * {self.ln.local_datatype_size}, &
                                       {self.ipn.datatype}, &
                                       {self.ipn.fh}, &
                                       {self.ln.local_request_id} )
end if
call SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} )
'''


class TaskIoSplitBegin(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskIoBegin}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('datatype', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ipn.ierror} == MPI_SUCCESS .and. {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call scorep_mpi_io_split_begin({self.ln.local_io_handle}, {self.ln.local_request_id}, {self.ipn.datatype})
    call SCOREP_IoOperationIssued( {self.ln.local_io_handle}, {self.ln.local_request_id} )
end if
call SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} )
'''


class TaskIoSplitEnd(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskGetIoHandle}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('status', 'ierror')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_count',
                    type_name='integer',
                    type_subtype='MPI_COUNT_KIND' if self.scp_procedure.is_embiggened() else None),
                initial_value='0'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_datatype',
                    type_name='type',
                    type_subtype='MPI_Datatype'),
                initial_value='MPI_DATATYPE_NULL'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_request_id',
                    type_name='integer',
                    type_subtype='SCOREP_MpiRequestId'))
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS', 'PMPI_Get_count', 'PMPI_Type_free'}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ipn.ierror} .eq. MPI_SUCCESS .and. {self.ln.local_io_handle} .ne. SCOREP_INVALID_IO_HANDLE ) then
    call scorep_mpi_io_split_end( {self.ln.local_io_handle}, {self.ln.local_request_id}, {self.ln.local_datatype} )
    call PMPI_Get_count( {self.ipn.status}, {self.ln.local_datatype}, {self.ln.local_count} )
    call SCOREP_IoOperationComplete( {self.ln.local_io_handle}, &
                                     {self.operation_mode()}, &
                                     {self.ln.local_count} * {self.get_datatype_size_fun()}({self.ln.local_datatype}), &
                                     {self.ln.local_request_id} )
#if {HAVE('MPI_2_0_SYMBOL_PMPI_TYPE_DUP')}
    call PMPI_Type_free( {self.ln.local_datatype} )
#endif
end if

call SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} )
'''
