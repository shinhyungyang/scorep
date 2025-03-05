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
           'TaskRmaBytes',
           'TaskRmaGet',
           'TaskRmaPut',
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

from typing import Type, Set

from wrapgen.data.scorep.rma import scorep_completion_type, CompletionType, scorep_atomic_type, AtomicType, \
    RequestCallback, scorep_epoch_type, EpochType
from wrapgen.language.fortran import FortranVariable, FortranArgument
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskWinCreate(F08Task):
    def collective_type(self) -> str:
        return 'SCOREP_COLLECTIVE_CREATE_HANDLE'

    def window_name(self) -> str:
        return f'{self.scp_procedure.std_name}_window'

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('comm', 'win')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_win_handle',
                    type_name='integer',
                    type_subtype='SCOREP_RmaWindowHandle'),
                initial_value='SCOREP_INVALID_RMA_WINDOW'
            )
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_WIN_NULL'}

    def generate_comment(self) -> GeneratorOutput:
        yield '''\
!> Triggers five events: Enter, RmaCollectiveBegin, WinCreate,
!> RmaCollectiveEnd, and Leave.
'''

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_RmaCollectiveBegin( SCOREP_RMA_SYNC_LEVEL_PROCESS )\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.win} .ne. MPI_WIN_NULL) then
    {self.ln.local_win_handle} = scorep_mpi_win_create( "{self.window_name()}", {self.ipn.win}, {self.ipn.comm} )
    call SCOREP_RmaWinCreate( {self.ln.local_win_handle} )
end if
'''
        yield from self.before_collective_end()
        yield f'''\
call SCOREP_RmaCollectiveEnd( {self.collective_type()}, &
                              SCOREP_RMA_SYNC_LEVEL_PROCESS, &
                              {self.ln.local_win_handle}, SCOREP_INVALID_ROOT_RANK, 0_c_int64_t, 0_c_int64_t )
'''

    def before_collective_end(self) -> GeneratorOutput:
        yield from ()


class TaskWinAllocate(TaskWinCreate):
    def collective_type(self) -> str:
        return 'SCOREP_COLLECTIVE_CREATE_HANDLE_AND_ALLOCATE'

    def needs_local_variables(self) -> Scope[FortranVariable]:
        my_locals = super().needs_local_variables()
        my_locals.update(Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_size_as_int64',
                    type_name='integer',
                    type_subtype='c_int64_t',
                    attributes=['target']),
                initial_value='0')))
        return my_locals

    def needs_parameter_names(self) -> Scope[str]:
        parameter_names = super().needs_parameter_names()
        parameter_names.update(self.make_parameter_name_mapping('baseptr', 'size', 'ierror'))
        return parameter_names

    def needs_mpi_f08_uses(self) -> set[str]:
        uses = super().needs_mpi_f08_uses()
        uses.update({'MPI_SUCCESS'})
        return uses

    def generate_enter_if_group_active_pre(self) -> GeneratorOutput:
        yield f'''\
if ( scorep_mpi_memory_recording ) then
    {self.ln.local_size_as_int64} = int({self.ipn.size}, c_int64_t)
    call SCOREP_AddAttribute( scorep_mpi_memory_alloc_size_attribute, c_loc({self.ln.local_size_as_int64}) )
end if
'''

    def before_collective_end(self) -> GeneratorOutput:
        yield f'''\
if ( scorep_mpi_memory_recording .and. {self.ipn.size} .gt. 0 .and. MPI_SUCCESS .eq. {self.ipn.ierror} ) then
    call SCOREP_AllocMetric_HandleAlloc( scorep_mpi_allocations_metric, &
                                        {self.ipn.baseptr}, &
                                        int({self.ipn.size}, c_size_t) )
end if
'''


class TaskWinFree(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_win_handle',
                    type_name='integer',
                    type_subtype='SCOREP_RmaWindowHandle'),
                initial_value='SCOREP_INVALID_RMA_WINDOW'
            ),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_collective_type',
                    type_name='integer',
                    type_subtype='SCOREP_CollectiveType'),
                initial_value='SCOREP_COLLECTIVE_DESTROY_HANDLE'
            )
        )

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_RmaCollectiveBegin( SCOREP_RMA_SYNC_LEVEL_PROCESS )\n'
        yield f'{self.ln.local_collective_type} = scorep_mpi_win_collective_type({self.ipn.win})\n'

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
{self.ln.local_win_handle} = scorep_mpi_win_handle( {self.ipn.win} )
call scorep_mpi_win_free( {self.ipn.win} )
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_RmaWinDestroy( {self.ln.local_win_handle} )
call SCOREP_RmaCollectiveEnd( {self.ln.local_collective_type}, &
                              SCOREP_RMA_SYNC_LEVEL_PROCESS, &
                              {self.ln.local_win_handle}, SCOREP_INVALID_ROOT_RANK, 0_c_int64_t, 0_c_int64_t )
'''


class TaskRmaRequestTrackingBase(F08Task):
    def rma_remote_completion(self) -> str:
        return scorep_completion_type(self.get_attribute('rma_completion_type', CompletionType))

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_rma_request',
                    type_name='type',
                    type_subtype='c_ptr'),
                initial_value='c_null_ptr'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_win_handle',
                    type_name='integer',
                    type_subtype='SCOREP_RmaWindowHandle')),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_matching_id',
                    type_name='integer',
                    type_subtype='SCOREP_MpiRequestId'),
                initial_value='0')
        )

    def generate_initialization(self) -> GeneratorOutput:
        yield f'{self.ln.local_win_handle} = scorep_mpi_win_handle( {self.ipn.win} )\n'


class TaskRmaRequestTrackingExplicit(TaskRmaRequestTrackingBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'target_rank', 'request')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_matching_id} = scorep_mpi_get_request_id()\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL) then
    ! Start implicit request tracking
    {self.ln.local_rma_request} = &
        scorep_mpi_rma_request_create( {self.ln.local_win_handle}, &
                                       {self.ipn.target_rank}, &
                                       {self.ipn.request}, &
                                       {self.rma_remote_completion()}, &
                                       {self.ln.local_matching_id} )
    ! Start explicit request tracking
    call scorep_mpi_request_win_create( {self.ipn.request}, {self.ln.local_rma_request} )
end if
'''


class TaskRmaRequestTrackingImplicit(TaskRmaRequestTrackingBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'target_rank')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL', 'MPI_REQUEST_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    ! Implicit request may already have a valid matching id
    {self.ln.local_rma_request} = scorep_mpi_rma_request_find( {self.ln.local_win_handle}, &
                                                               {self.ipn.target_rank}, &
                                                               MPI_REQUEST_NULL, &
                                                               {self.rma_remote_completion()} )
    if ( .not. c_associated({self.ln.local_rma_request}) ) then
        {self.ln.local_matching_id} = scorep_mpi_get_request_id()
    else
        {self.ln.local_matching_id} = scorep_mpi_rma_request_matching_id({self.ln.local_rma_request})
    end if
end if
'''

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ({self.ipn.target_rank} .ne. MPI_PROC_NULL) then
    ! Only implicit request tracking needed
    if (.not. c_associated({self.ln.local_rma_request})) then
        {self.ln.local_rma_request} =  scorep_mpi_rma_request_create( {self.ln.local_win_handle}, &
                                                                      {self.ipn.target_rank}, &
                                                                      MPI_REQUEST_NULL, &
                                                                      {self.rma_remote_completion()}, &
                                                                      {self.ln.local_matching_id} )
    end if
end if
'''


class TaskRmaBytes(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'origin_datatype', 'origin_count')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_origin_bytes',
                    type_name='integer',
                    type_subtype='c_int64_t'),
                initial_value='0_c_int64_t')
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    {self.ln.local_origin_bytes} = {self.ipn.origin_count} * {self.get_datatype_size_fun()}({self.ipn.origin_datatype})
end if
'''


class TaskRmaGet(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskRmaRequestTrackingBase, TaskRmaBytes}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    call SCOREP_RmaGet( {self.ln.local_win_handle}, {self.ipn.target_rank}, &
                        {self.ln.local_origin_bytes}, &
                        {self.ln.local_matching_id} )
end if
'''


class TaskRmaPut(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskRmaRequestTrackingBase, TaskRmaBytes}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    call SCOREP_RmaPut( {self.ln.local_win_handle}, {self.ipn.target_rank}, &
                        {self.ln.local_origin_bytes}, &
                        {self.ln.local_matching_id} )
end if
'''


class TaskRmaAtomicBytesBase(F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_bytes_put',
                    type_name='integer',
                    type_subtype='c_int64_t'),
                initial_value='0'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_bytes_get',
                    type_name='integer',
                    type_subtype='c_int64_t'),
                initial_value='0')
        )

    def get_datatype_size_fun(self):
        fun_name = 'get_datatype_size'
        if self.scp_procedure.is_embiggened():
            fun_name += '_c'
        return fun_name


class TaskRmaAtomicBytesAccumulate(TaskRmaAtomicBytesBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'origin_count', 'origin_datatype')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    {self.ln.local_bytes_get} = 0
    {self.ln.local_bytes_put} = {self.ipn.origin_count} * {self.get_datatype_size_fun()}( {self.ipn.origin_datatype} )
end if
'''


class TaskRmaAtomicBytesGetAccumulate(TaskRmaAtomicBytesBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'origin_count', 'origin_datatype', 'result_count',
                                                'result_datatype')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    {self.ln.local_bytes_get} = {self.ipn.result_count} * {self.get_datatype_size_fun()}( {self.ipn.result_datatype} )
    {self.ln.local_bytes_put} = {self.ipn.origin_count} * {self.get_datatype_size_fun()}( {self.ipn.origin_datatype} )
end if
'''


class TaskRmaAtomicBytesScalar(TaskRmaAtomicBytesBase):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank', 'datatype')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    {self.ln.local_bytes_get} = {self.get_datatype_size_fun()}( {self.ipn.datatype} )
    {self.ln.local_bytes_put} = {self.get_datatype_size_fun()}( {self.ipn.datatype} )
end if
'''


class TaskRmaAtomic(F08Task):
    def atomic_type(self) -> str:
        return scorep_atomic_type(self.get_attribute('atomic_type', AtomicType))

    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskRmaRequestTrackingBase, TaskRmaAtomicBytesBase}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('target_rank')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_PROC_NULL'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.target_rank} .ne. MPI_PROC_NULL ) then
    call SCOREP_RmaAtomic( {self.ln.local_win_handle}, &
                           {self.ipn.target_rank}, &
                           {self.atomic_type()}, &
                           {self.ln.local_bytes_put}, &
                           {self.ln.local_bytes_get}, &
                           {self.ln.local_matching_id} )
end if
'''


class TaskWinLock(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'rank', 'lock_type')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_LOCK_EXCLUSIVE'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_RmaRequestLock( scorep_mpi_win_handle( {self.ipn.win} ), &
                           {self.ipn.rank}, &
                           0_c_int64_t, &
                           merge(SCOREP_LOCK_EXCLUSIVE, SCOREP_LOCK_SHARED, {self.ipn.lock_type} .eq. MPI_LOCK_EXCLUSIVE) )
'''


class TaskWinLockAll(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_RmaRequestLock( scorep_mpi_win_handle( {self.ipn.win} ), &
                            SCOREP_ALL_TARGET_RANKS, &
                            0_c_int64_t, &
                            SCOREP_LOCK_SHARED )
'''


class TaskWinHandleAtExit(AuxiliaryTaskMixin, F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_win_handle',
                    type_name='integer',
                    type_subtype='SCOREP_RmaWindowHandle'))
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'{self.ln.local_win_handle} = scorep_mpi_win_handle({self.ipn.win})\n'


class TaskRmaRequestForeachToTarget(F08Task):
    def completion_callback(self) -> str:
        return self.get_attribute('callback', RequestCallback).value

    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('rank')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call scorep_mpi_rma_request_foreach_to_target( {self.ln.local_win_handle}, &
                                               {self.ipn.rank}, &
                                               c_funloc({self.completion_callback()}) )
'''


class TaskRmaRequestForeachOnWindow(F08Task):
    def completion_callback(self) -> str:
        return self.get_attribute('callback', RequestCallback).value

    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call scorep_mpi_rma_request_foreach_on_window( {self.ln.local_win_handle}, &
                                               c_funloc({self.completion_callback()}) )
'''


class TaskWinUnlock(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachToTarget}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'rank')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'call SCOREP_RmaReleaseLock( {self.ln.local_win_handle}, {self.ipn.rank}, 0_c_int64_t )\n'


class TaskWinUnlockAll(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachOnWindow}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'call SCOREP_RmaReleaseLock( {self.ln.local_win_handle}, SCOREP_ALL_TARGET_RANKS, 0_c_int64_t )\n'


class TaskWinFence(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachOnWindow}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_RmaCollectiveBegin( ior(SCOREP_RMA_SYNC_LEVEL_MEMORY,SCOREP_RMA_SYNC_LEVEL_PROCESS) )\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
 ! TODO: evaluation of assert could lead to better value for the SYNC_LEVEL
call SCOREP_RmaCollectiveEnd( SCOREP_COLLECTIVE_BARRIER, &
                              ior(SCOREP_RMA_SYNC_LEVEL_MEMORY,SCOREP_RMA_SYNC_LEVEL_PROCESS), &
                              {self.ln.local_win_handle}, &
                              SCOREP_INVALID_ROOT_RANK, &
                              0_c_int64_t, 0_c_int64_t )
'''


class TaskEpochStart(F08Task):
    def epoch_type(self) -> str:
        return scorep_epoch_type(self.get_attribute('epoch_type', EpochType))

    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'group')

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield f'call scorep_mpi_epoch_start({self.ipn.win}, {self.ipn.group}, {self.epoch_type()} )\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''call SCOREP_RmaGroupSync( SCOREP_RMA_SYNC_LEVEL_PROCESS, &
                                            {self.ln.local_win_handle}, &
                                            scorep_mpi_group_handle( {self.ipn.group} ) )
               '''


class TaskEpochEnd(F08Task):
    def epoch_type(self) -> str:
        return scorep_epoch_type(self.get_attribute('epoch_type', EpochType))

    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit, TaskRmaRequestForeachOnWindow}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win')

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_RmaGroupSync( ior(SCOREP_RMA_SYNC_LEVEL_MEMORY,SCOREP_RMA_SYNC_LEVEL_PROCESS), &
                          {self.ln.local_win_handle}, &
                          scorep_mpi_epoch_get_group_handle( {self.ipn.win}, {self.epoch_type()} ) )
                          call scorep_mpi_epoch_end( {self.ipn.win}, {self.epoch_type()} )
'''


class TaskWinTest(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'flag')

    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskWinHandleAtExit}

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
if ({self.ipn.flag}) then
    call scorep_mpi_rma_request_foreach_on_window( {self.ln.local_win_handle}, &
                                                   c_funloc(scorep_mpi_rma_request_write_standard_completion) )
    call SCOREP_RmaGroupSync( ior(SCOREP_RMA_SYNC_LEVEL_MEMORY,SCOREP_RMA_SYNC_LEVEL_PROCESS), &
                              {self.ln.local_win_handle}, &
                              scorep_mpi_epoch_get_group_handle( {self.ipn.win}, SCOREP_MPI_RMA_EXPOSURE_EPOCH ) )
    call scorep_mpi_epoch_end({self.ipn.win}, SCOREP_MPI_RMA_EXPOSURE_EPOCH )
else
    call SCOREP_RmaGroupSync( SCOREP_RMA_SYNC_LEVEL_NONE, &
                              {self.ln.local_win_handle}, &
                              scorep_mpi_epoch_get_group_handle( {self.ipn.win}, SCOREP_MPI_RMA_EXPOSURE_EPOCH ) )
end if
'''


class TaskWinSetName(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('win', 'win_name', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.ierror} .eq. MPI_SUCCESS ) then
    call scorep_mpi_win_set_name( {self.ipn.win}, {self.ipn.win_name} )
end if
'''
