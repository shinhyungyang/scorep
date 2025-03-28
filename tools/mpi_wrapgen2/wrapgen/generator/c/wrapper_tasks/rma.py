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
__all__ = ['TaskWinCreate',
           'TaskWinAllocate',
           'TaskWinFree',
           'TaskRmaRequestTrackingBase',
           'TaskRmaRequestTrackingExplicit',
           'TaskRmaRequestTrackingImplicit',
           'TaskRmaGet',
           'TaskRmaPut',
           'TaskRmaBytes',
           'TaskRmaAtomicBytesBase',
           'TaskRmaAtomicBytesAccumulate',
           'TaskRmaAtomicBytesGetAccumulate',
           'TaskRmaAtomicBytesScalar',
           'TaskRmaAtomic',
           'TaskWinLock',
           'TaskWinLockAll',
           'TaskWinHandleAtExit',
           'TaskRmaRequestForeachToTarget',
           'TaskRmaRequestForeachOnWindow',
           'TaskWinUnlock',
           'TaskWinUnlockAll',
           'TaskWinFence',
           'TaskEpochStart',
           'TaskEpochEnd',
           'TaskWinTest',
           'TaskWinSetName']

from typing import Set, Type

from wrapgen.data.scorep.rma import scorep_completion_type, scorep_atomic_type, \
    scorep_epoch_type, CompletionType, AtomicType, RequestCallback, EpochType
from wrapgen.language.iso_c import CVariable, CParameter
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskWinCreate(CTask):
    def collective_type(self) -> str:
        return 'SCOREP_COLLECTIVE_CREATE_HANDLE'

    def window_name(self) -> str:
        return f'{self.scp_procedure.std_name}_window'

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'win')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_win_handle', base_type='SCOREP_RmaWindowHandle'),
                initial_value='SCOREP_INVALID_RMA_WINDOW'
            )
        )

    def generate_comment(self) -> GeneratorOutput:
        yield '''\
* Triggers five events: Enter, RmaCollectiveBegin, WinCreate,
* RmaCollectiveEnd, and Leave.
'''

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'SCOREP_RmaCollectiveBegin( SCOREP_RMA_SYNC_LEVEL_PROCESS );\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if (*{self.ipn.win} != MPI_WIN_NULL)
{{
    {self.ln.local_win_handle} = scorep_mpi_win_create( "{self.window_name()}", *{self.ipn.win}, {self.ipn.comm} );
    SCOREP_RmaWinCreate( {self.ln.local_win_handle} );
}}
'''
        yield from self.before_collective_end()
        yield f'''\
SCOREP_RmaCollectiveEnd( {self.collective_type()},
                         SCOREP_RMA_SYNC_LEVEL_PROCESS,
                         {self.ln.local_win_handle}, SCOREP_INVALID_ROOT_RANK, 0, 0 );
'''

    def before_collective_end(self) -> GeneratorOutput:
        yield from ()


class TaskWinAllocate(TaskWinCreate):
    def collective_type(self) -> str:
        return 'SCOREP_COLLECTIVE_CREATE_HANDLE_AND_ALLOCATE'

    def needs_parameter_names(self) -> Scope[str]:
        parameter_names = super().needs_parameter_names()
        parameter_names.update(self.make_parameter_name_mapping('baseptr', 'size'))
        return parameter_names

    def generate_enter_if_group_active_pre(self) -> GeneratorOutput:
        yield f'''\
if ( scorep_mpi_memory_recording )
{{
    uint64_t size_as_uint64 = {self.ipn.size};
    SCOREP_AddAttribute( scorep_mpi_memory_alloc_size_attribute,
                         &size_as_uint64 );
}}
'''

    def before_collective_end(self) -> GeneratorOutput:
        yield f'''\
if ( scorep_mpi_memory_recording && {self.ipn.size} > 0 && MPI_SUCCESS == {self.ln.return_value} )
{{
    SCOREP_AllocMetric_HandleAlloc( scorep_mpi_allocations_metric,
                                    ( uint64_t )( *( void** ){self.ipn.baseptr} ),
                                    {self.ipn.size} );
}}
'''


class TaskWinFree(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='local_win_handle', base_type='SCOREP_RmaWindowHandle')
            ),
            CVariable(
                parameter=CParameter(name='local_collective_type', base_type='SCOREP_CollectiveType'),
                initial_value='SCOREP_COLLECTIVE_DESTROY_HANDLE'
            )
        )

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'SCOREP_RmaCollectiveBegin( SCOREP_RMA_SYNC_LEVEL_PROCESS );\n'
        yield f'{self.ln.local_collective_type} = scorep_mpi_win_collective_type( *{self.ipn.win} );\n'

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
 // save handle reference for RmaCollectiveEnd
{self.ln.local_win_handle} = scorep_mpi_win_handle( *{self.ipn.win} );
 // stop tracking window
scorep_mpi_win_free( *{self.ipn.win} );
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'SCOREP_RmaWinDestroy( {self.ln.local_win_handle} );\n'
        yield f'''\
SCOREP_RmaCollectiveEnd( {self.ln.local_collective_type},
                         SCOREP_RMA_SYNC_LEVEL_PROCESS,
                         {self.ln.local_win_handle}, SCOREP_INVALID_ROOT_RANK, 0, 0 );
'''


class TaskRmaRequestTrackingBase(CTask):
    def rma_remote_completion(self) -> str:
        return scorep_completion_type(self.get_attribute('rma_completion_type', CompletionType))

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(
                    name='local_rma_request',
                    base_type='scorep_mpi_rma_request',
                    pointer_level=1),
                initial_value='NULL'),
            CVariable(
                parameter=CParameter(
                    name='local_win_handle',
                    base_type='SCOREP_RmaWindowHandle')),
            CVariable(
                parameter=CParameter(
                    name='local_matching_id',
                    base_type='SCOREP_MpiRequestId'),
                initial_value='0')
        )

    def generate_initialization(self) -> GeneratorOutput:
        yield f'{self.ln.local_win_handle} = scorep_mpi_win_handle( {self.ipn.win} );\n'


class TaskRmaRequestTrackingExplicit(TaskRmaRequestTrackingBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'target_rank', 'request')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_matching_id} = scorep_mpi_get_request_id();\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ({self.ipn.target_rank} != MPI_PROC_NULL)
{{
    // Start implicit request tracking
    {self.ln.local_rma_request} = scorep_mpi_rma_request_create( {self.ln.local_win_handle},
                                                                 {self.ipn.target_rank},
                                                                 *{self.ipn.request},
                                                                 {self.rma_remote_completion()},
                                                                 {self.ln.local_matching_id} );
    // Start explicit request tracking
    scorep_mpi_request_win_create( *{self.ipn.request}, {self.ln.local_rma_request} );
}}
'''


class TaskRmaRequestTrackingImplicit(TaskRmaRequestTrackingBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'target_rank')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ({self.ipn.target_rank} != MPI_PROC_NULL)
{{
    // Implicit request may already have a valid matching id
    {self.ln.local_rma_request} = scorep_mpi_rma_request_find( {self.ln.local_win_handle},
                                                               {self.ipn.target_rank},
                                                               MPI_REQUEST_NULL,
                                                               {self.rma_remote_completion()} );

    {self.ln.local_matching_id} = ( {self.ln.local_rma_request} == NULL )
        ? scorep_mpi_get_request_id()
        : {self.ln.local_rma_request}->matching_id;
}}
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    // Only implicit request tracking needed
    if ( {self.ln.local_rma_request} == NULL )
    {{
        scorep_mpi_rma_request_create( {self.ln.local_win_handle},
                                       {self.ipn.target_rank},
                                       MPI_REQUEST_NULL,
                                       {self.rma_remote_completion()},
                                       {self.ln.local_matching_id} );
    }}
}}
'''


class TaskRmaBytes(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'origin_datatype', 'origin_count')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter('local_origin_bytes', base_type='uint64_t'), initial_value='0')
        )

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    {self.ln.local_origin_bytes} = {self.ipn.origin_count} * {self.get_datatype_size_fun()}({self.ipn.origin_datatype});
}}
'''


class TaskRmaGet(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskRmaRequestTrackingBase, TaskRmaBytes}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    SCOREP_RmaGet( {self.ln.local_win_handle}, {self.ipn.target_rank},
                   {self.ln.local_origin_bytes},
                   {self.ln.local_matching_id} );
}}
'''


class TaskRmaPut(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskRmaRequestTrackingBase, TaskRmaBytes}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    SCOREP_RmaPut( {self.ln.local_win_handle}, {self.ipn.target_rank},
                   {self.ln.local_origin_bytes},
                   {self.ln.local_matching_id} );
}}
'''


class TaskRmaAtomicBytesBase(CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter('local_bytes_put', base_type='uint64_t'), initial_value='0'),
            CVariable(parameter=CParameter('local_bytes_get', base_type='uint64_t'), initial_value='0')
        )

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name


class TaskRmaAtomicBytesAccumulate(TaskRmaAtomicBytesBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'origin_count', 'origin_datatype')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    {self.ln.local_bytes_get} = 0;
    {self.ln.local_bytes_put} = {self.ipn.origin_count} * {self.get_datatype_size_fun()}( {self.ipn.origin_datatype} );
}}
'''


class TaskRmaAtomicBytesGetAccumulate(TaskRmaAtomicBytesBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'origin_count', 'origin_datatype', 'result_count',
                                                'result_datatype')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    {self.ln.local_bytes_get} = {self.ipn.result_count} * {self.get_datatype_size_fun()}( {self.ipn.result_datatype} );
    {self.ln.local_bytes_put} = {self.ipn.origin_count} * {self.get_datatype_size_fun()}( {self.ipn.origin_datatype} );
}}
'''


class TaskRmaAtomicBytesScalar(TaskRmaAtomicBytesBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'datatype')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    {self.ln.local_bytes_get} = {self.get_datatype_size_fun()}( {self.ipn.datatype} );
    {self.ln.local_bytes_put} = {self.get_datatype_size_fun()}( {self.ipn.datatype} );
}}
'''


class TaskRmaAtomic(CTask):
    def atomic_type(self) -> str:
        return scorep_atomic_type(self.get_attribute('atomic_type', AtomicType))

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskRmaRequestTrackingBase, TaskRmaAtomicBytesBase}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} != MPI_PROC_NULL )
{{
    SCOREP_RmaAtomic( {self.ln.local_win_handle},
                      {self.ipn.target_rank},
                      {self.atomic_type()},
                      {self.ln.local_bytes_put},
                      {self.ln.local_bytes_get},
                      {self.ln.local_matching_id} );
}}
'''


class TaskWinLock(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'rank', 'lock_type')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
SCOREP_RmaRequestLock( scorep_mpi_win_handle( {self.ipn.win} ), {self.ipn.rank}, 0,
                       {self.ipn.lock_type} == MPI_LOCK_EXCLUSIVE ? SCOREP_LOCK_EXCLUSIVE : SCOREP_LOCK_SHARED );
'''


class TaskWinLockAll(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'SCOREP_RmaRequestLock( scorep_mpi_win_handle( {self.ipn.win} ), SCOREP_ALL_TARGET_RANKS, 0, SCOREP_LOCK_SHARED );\n'


class TaskWinHandleAtExit(AuxiliaryTaskMixin, CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_win_handle', base_type='SCOREP_RmaWindowHandle'))
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_win_handle} = scorep_mpi_win_handle({self.ipn.win});\n'


class TaskRmaRequestForeachToTarget(CTask):
    def completion_callback(self) -> str:
        return self.get_attribute('callback', RequestCallback).value

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('rank')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
scorep_mpi_rma_request_foreach_to_target( {self.ln.local_win_handle},
                                          {self.ipn.rank},
                                          {self.completion_callback()} );
'''


class TaskRmaRequestForeachOnWindow(CTask):
    def completion_callback(self) -> str:
        return self.get_attribute('callback', RequestCallback).value

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
scorep_mpi_rma_request_foreach_on_window( {self.ln.local_win_handle},
                                          {self.completion_callback()} );
'''


class TaskWinUnlock(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachToTarget}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'rank')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'SCOREP_RmaReleaseLock( {self.ln.local_win_handle}, {self.ipn.rank}, 0 );\n'


class TaskWinUnlockAll(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachOnWindow}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'SCOREP_RmaReleaseLock( {self.ln.local_win_handle}, SCOREP_ALL_TARGET_RANKS, 0 );\n'


class TaskWinFence(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachOnWindow}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield '''\
SCOREP_RmaCollectiveBegin( SCOREP_RMA_SYNC_LEVEL_MEMORY |
                           SCOREP_RMA_SYNC_LEVEL_PROCESS );
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
 // TODO: evaluation of assert could lead to better value for the SYNC_LEVEL
SCOREP_RmaCollectiveEnd( SCOREP_COLLECTIVE_BARRIER,
                         SCOREP_RMA_SYNC_LEVEL_MEMORY | SCOREP_RMA_SYNC_LEVEL_PROCESS,
                         {self.ln.local_win_handle},
                         SCOREP_INVALID_ROOT_RANK,
                         0, 0 );
'''


class TaskEpochStart(CTask):
    def epoch_type(self) -> str:
        return scorep_epoch_type(self.get_attribute('epoch_type', EpochType))

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'group')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'scorep_mpi_epoch_start({self.ipn.win}, {self.ipn.group}, {self.epoch_type()} );\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
SCOREP_RmaGroupSync( SCOREP_RMA_SYNC_LEVEL_PROCESS,
                     {self.ln.local_win_handle},
                     scorep_mpi_group_handle( {self.ipn.group} ) );
'''


class TaskEpochEnd(CTask):
    def epoch_type(self) -> str:
        return scorep_epoch_type(self.get_attribute('epoch_type', EpochType))

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachOnWindow}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
SCOREP_RmaGroupSync( SCOREP_RMA_SYNC_LEVEL_MEMORY | SCOREP_RMA_SYNC_LEVEL_PROCESS,
                     {self.ln.local_win_handle},
                     scorep_mpi_epoch_get_group_handle( {self.ipn.win}, {self.epoch_type()} ) );
                     scorep_mpi_epoch_end( {self.ipn.win}, {self.epoch_type()} );
'''


class TaskWinTest(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'flag')

    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskWinHandleAtExit}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( *{self.ipn.flag} != 0 )
{{
    scorep_mpi_rma_request_foreach_on_window( {self.ln.local_win_handle},
                                              scorep_mpi_rma_request_write_standard_completion );

    SCOREP_RmaGroupSync( SCOREP_RMA_SYNC_LEVEL_MEMORY | SCOREP_RMA_SYNC_LEVEL_PROCESS,
                         {self.ln.local_win_handle},
                         scorep_mpi_epoch_get_group_handle( {self.ipn.win}, SCOREP_MPI_RMA_EXPOSURE_EPOCH ) );
    scorep_mpi_epoch_end( {self.ipn.win}, SCOREP_MPI_RMA_EXPOSURE_EPOCH );
}}
else
{{
    SCOREP_RmaGroupSync( SCOREP_RMA_SYNC_LEVEL_NONE,
                         {self.ln.local_win_handle},
                         scorep_mpi_epoch_get_group_handle( {self.ipn.win}, SCOREP_MPI_RMA_EXPOSURE_EPOCH ) );
}}
'''


class TaskWinSetName(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'win_name')

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ln.return_value} == MPI_SUCCESS )
{{
    scorep_mpi_win_set_name( {self.ipn.win}, {self.ipn.win_name} );
}}
'''
