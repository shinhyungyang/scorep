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

from wrapgen.data.scorep.io import OperationMode, scorep_operation_mode, \
    scorep_blocking_operation_flag, scorep_collective_operation_flag
from wrapgen.language.iso_c import CVariable, CParameter
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.c.wrapper_tasks import CTask, TaskGetRequestId
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskFileOpen(CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_comm_handle', base_type='SCOREP_InterimCommunicatorHandle')
            ),
            CVariable(
                parameter=CParameter(name='local_unify_key', base_type='uint32_t'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='local_file', base_type='SCOREP_IoFileHandle')
            ),
            CVariable(
                parameter=CParameter(name='local_io_handle', base_type='SCOREP_IoHandleHandle')
            )
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'filename', 'amode', 'fh')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
{self.ln.local_comm_handle} = SCOREP_MPI_COMM_HANDLE({self.ipn.comm});\n
{self.ln.local_unify_key} = scorep_mpi_io_get_unify_key( {self.ln.local_comm_handle} );
SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_MPI,
                                   SCOREP_IO_HANDLE_FLAG_NONE,
                                   {self.ln.local_comm_handle},
                                   "" );
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.return_value} == MPI_SUCCESS )
{{
    {self.ln.local_file} = SCOREP_IoMgmt_GetIoFileHandle( {self.ipn.filename} );
    {self.ln.local_io_handle} = SCOREP_IoMgmt_CompleteHandleCreation(SCOREP_IO_PARADIGM_MPI,
                                                                     {self.ln.local_file},
                                                                     {self.ln.local_unify_key},
                                                                     {self.ipn.fh} );
    if( {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
    {{
        SCOREP_IoCreateHandle( {self.ln.local_io_handle},
                               scorep_mpi_io_get_access_mode( {self.ipn.amode} ),
                               scorep_mpi_io_get_creation_flags( {self.ipn.amode} ),
                               scorep_mpi_io_get_status_flags( {self.ipn.amode} ) );
    }}
}}
else
{{
    SCOREP_IoMgmt_DropIncompleteHandle();
}}
'''


class TaskFileClose(CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_amode', base_type='int')),
            CVariable(
                parameter=CParameter(name='local_io_handle', base_type='SCOREP_IoHandleHandle'),
                initial_value='SCOREP_INVALID_IO_HANDLE'
            ),
            CVariable(
                parameter=CParameter(name='local_file', base_type='SCOREP_IoFileHandle')
            ),
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('fh')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
{self.ln.local_io_handle} = SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_MPI, {self.ipn.fh} );
PMPI_File_get_amode( *{self.ipn.fh}, &{self.ln.local_amode});
SCOREP_IoMgmt_PushHandle( {self.ln.local_io_handle} );
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
    SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} );
    if({self.ln.return_value} == MPI_SUCCESS)
    {{
        if( {self.ln.local_amode} & MPI_MODE_DELETE_ON_CLOSE )
        {{
            {self.ln.local_file} = SCOREP_IoHandleHandle_GetIoFile( {self.ln.local_io_handle} );
            SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_MPI, {self.ln.local_file} );
        }}
        SCOREP_IoDestroyHandle( {self.ln.local_io_handle} );
        SCOREP_IoMgmt_DestroyHandle( {self.ln.local_io_handle} );
    }}
    else
    {{
        SCOREP_IoMgmt_ReinsertHandle( SCOREP_IO_PARADIGM_MPI, {self.ln.local_io_handle} );
    }}
}}
'''


class TaskFileSeek(CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_io_handle', base_type='SCOREP_IoHandleHandle'),
                initial_value='SCOREP_INVALID_IO_HANDLE'
            ),
            CVariable(
                parameter=CParameter(name='local_pos', base_type='MPI_Offset')
            )
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('fh', 'offset', 'whence')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_io_handle} = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_MPI, &{self.ipn.fh} );\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
    PMPI_File_get_position( {self.ipn.fh}, &{self.ln.local_pos} );

    SCOREP_IoSeek( {self.ln.local_io_handle},
                   {self.ipn.offset},
                   scorep_mpi_io_get_seek_option( {self.ipn.whence} ),
                   {self.ln.local_pos} );

    SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} );
}}
'''


class TaskFileDelete(CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_file', base_type='SCOREP_IoFileHandle'),
                initial_value='SCOREP_INVALID_IO_FILE'
            ),
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('filename')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_file} = SCOREP_IoMgmt_GetIoFileHandle({self.ipn.filename});\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.local_file} != SCOREP_INVALID_IO_FILE )
{{
    SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_MPI, {self.ln.local_file} );
}}
'''


class TaskGetIoHandle(AuxiliaryTaskMixin, CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_io_handle', base_type='SCOREP_IoHandleHandle'),
                initial_value='SCOREP_INVALID_IO_HANDLE'
            )
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('fh')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_io_handle} = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_MPI, &{self.ipn.fh} );\n'


class TaskGetDatatypeSize(AuxiliaryTaskMixin, CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_datatype_size', base_type='uint64_t'),
                initial_value='0'
            )
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('datatype')

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_datatype_size} = {self.get_datatype_size_fun()}({self.ipn.datatype});\n'


class TaskIoBegin(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
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
if( {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
    SCOREP_IoOperationBegin( {self.ln.local_io_handle},
                             {self.operation_mode()},
                             {self.collective_flag()} | {self.blocking_flag()},
                             {self.ipn.count} * {self.ln.local_datatype_size},
                             {self.ln.local_request_id} /* matching id */,
                             {self.operation_offset()} );
}}
'''


class TaskIoBlocking(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskIoBegin}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def PMPI_Get_count_fun(self):
        fun_name = 'PMPI_Get_count'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('status', 'datatype')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_count',
                                           base_type='MPI_Count' if self.scp_procedure.is_embiggened() else 'int'),
                      initial_value='0')
        )

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
   {self.PMPI_Get_count_fun()}( {self.ipn.status}, {self.ipn.datatype}, &{self.ln.local_count} );
   SCOREP_IoOperationComplete( {self.ln.local_io_handle},
                               {self.operation_mode()},
                               (uint64_t) {self.ln.local_count} * {self.ln.local_datatype_size},
                               {self.ln.local_request_id} );
}}
SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} );
'''


class TaskIoNonBlocking(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskIoBegin}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request', 'datatype', 'count', 'fh')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.return_value} == MPI_SUCCESS && {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
    SCOREP_IoOperationIssued( {self.ln.local_io_handle}, {self.ln.local_request_id} );

    scorep_mpi_request_io_create( *{self.ipn.request},
                                  {self.operation_mode()},
                                  {self.ipn.count} * {self.ln.local_datatype_size},
                                  {self.ipn.datatype},
                                  {self.ipn.fh},
                                  {self.ln.local_request_id} );
}}
SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} );
'''


class TaskIoSplitBegin(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskIoBegin}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('datatype')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.return_value} == MPI_SUCCESS && {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
    scorep_mpi_io_split_begin({self.ln.local_io_handle}, {self.ln.local_request_id}, {self.ipn.datatype});
    SCOREP_IoOperationIssued( {self.ln.local_io_handle}, {self.ln.local_request_id} );
}}
SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} );
'''


class TaskIoSplitEnd(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskGetIoHandle}

    def operation_mode(self) -> str:
        return scorep_operation_mode(self.get_attribute('mode', OperationMode))

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def PMPI_Get_count_fun(self):
        fun_name = 'PMPI_Get_count'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('status')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_count',
                                           base_type='MPI_Count' if self.scp_procedure.is_embiggened() else 'int'),
                      initial_value='0'),
            CVariable(parameter=CParameter(name='local_datatype', base_type='MPI_Datatype'),
                      initial_value='MPI_DATATYPE_NULL'),
            CVariable(parameter=CParameter(name='local_request_id', base_type='SCOREP_MpiRequestId'))
        )

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if( {self.ln.return_value} == MPI_SUCCESS && {self.ln.local_io_handle} != SCOREP_INVALID_IO_HANDLE )
{{
    scorep_mpi_io_split_end( {self.ln.local_io_handle}, &{self.ln.local_request_id}, &{self.ln.local_datatype} );

    {self.PMPI_Get_count_fun()}( {self.ipn.status}, {self.ln.local_datatype}, &{self.ln.local_count} );
    SCOREP_IoOperationComplete( {self.ln.local_io_handle},
                              {self.operation_mode()},
                              (uint64_t) {self.ln.local_count} * {self.get_datatype_size_fun()}({self.ln.local_datatype}),
                              {self.ln.local_request_id} );
#if HAVE( MPI_2_0_SYMBOL_PMPI_TYPE_DUP )
    PMPI_Type_free( &{self.ln.local_datatype} );
#endif
}}
SCOREP_IoMgmt_PopHandle( {self.ln.local_io_handle} );
'''
