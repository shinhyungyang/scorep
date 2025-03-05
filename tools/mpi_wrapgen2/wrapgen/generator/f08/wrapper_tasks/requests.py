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
__all__ = ['TaskWait',
           'TaskWaitany',
           'TaskWaitall',
           'TaskTestany',
           'TaskTestall',
           'TaskTest',
           'TaskSaveRequestArray',
           'TaskTestWaitSome',
           'TaskStart',
           'TaskStartall',
           'TaskRequestGetStatus',
           'TaskXreqtestEnabled',
           'TaskCancel',
           'TaskRequestFree']

from typing import Set, Type

from wrapgen.language.fortran import FortranArgument, FortranVariable
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope
from wrapgen.generator.f08 import scorep_f08_macros


class TaskSaveRequestArray(AuxiliaryTaskMixin, F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping(requests=['request', 'array_of_requests'])

    def generate_pre_pmpi_call(self):
        yield f'call scorep_mpi_save_request_array( {self.ipn.requests} ) \n'


class TaskXreqtestEnabled(AuxiliaryTaskMixin, F08Task):
    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='xreqtest_enabled',
                    type_name='logical'
                ),
                initial_value=f'{scorep_f08_macros.SCOREP_MPI_IS_EVENT_GEN_ON_FOR("XREQTEST")}'
            )
        )


class TaskWait(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('status')

    def generate_post_pmpi_call(self):
        yield f'call scorep_mpi_check_all_or_none( 1, .true., {self.ipn.status} )\n'


class TaskWaitall(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'array_of_statuses')

    def generate_post_pmpi_call(self):
        yield f'call scorep_mpi_check_all_or_none( {self.ipn.count}, .true., {self.ipn.array_of_statuses} )\n'


class TaskWaitany(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('index', 'count', 'status')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_UNDEFINED'}

    def generate_post_pmpi_call(self):
        yield f'''\
if ( {self.ipn.index} .ne. MPI_UNDEFINED ) then
    ! array_of_requests contains active handles
    if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
        call scorep_mpi_check_some_test_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} )
    else
        call scorep_mpi_check_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} )
    end if
end if
'''


class TaskTestWaitSome(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('incount', 'outcount', 'array_of_indices', 'array_of_statuses')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_UNDEFINED'}

    def generate_post_pmpi_call(self):
        yield f'''\
if ( {self.ipn.outcount} .ne. MPI_UNDEFINED ) then
    ! array_of_requests contains active handles
    if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
        call scorep_mpi_check_some_test_some( {self.ipn.incount}, {self.ipn.outcount}, {self.ipn.array_of_indices}, {self.ipn.array_of_statuses} )
    else
        call scorep_mpi_check_some( {self.ipn.incount}, {self.ipn.outcount}, {self.ipn.array_of_indices}, {self.ipn.array_of_statuses} )
    end if
end if
'''


class TaskTest(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('flag', 'status')

    def generate_post_pmpi_call(self):
        yield f'''\
if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
    call scorep_mpi_check_all_or_test_all( 1, {self.ipn.flag}, {self.ipn.status} )
else
    call scorep_mpi_check_all_or_none( 1, {self.ipn.flag}, {self.ipn.status} )
end if
'''


class TaskTestall(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'flag', 'array_of_statuses')

    def generate_post_pmpi_call(self):
        yield f'''\
if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
    call scorep_mpi_check_all_or_test_all( {self.ipn.count}, {self.ipn.flag}, {self.ipn.array_of_statuses} )
else
    call scorep_mpi_check_all_or_none( {self.ipn.count}, {self.ipn.flag}, {self.ipn.array_of_statuses} )
end if
'''


class TaskTestany(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'flag', 'status', 'index')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_UNDEFINED'}

    def generate_post_pmpi_call(self):
        yield f'''\
if ( .not. {self.ipn.flag} ) then
    ! array_of_requests contains active handles, but none were completed
    if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
        call scorep_mpi_test_all( {self.ipn.count} )
    end if
else if ( {self.ipn.index} .ne. MPI_UNDEFINED ) then
    ! array_of_requests contains active handles, and one was completed
    if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
        call scorep_mpi_check_some_test_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} )
    else
        call scorep_mpi_check_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} )
    end if
end if
 ! else, array_of_requests contains no active handles
'''


class TaskStart(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def generate_enter_if_group_active(self):
        yield f'call scorep_mpi_request_start({self.ipn.request})\n'


class TaskStartall(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'array_of_requests')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(parameter=FortranArgument.new(name='local_i', type_name='integer'))
        )

    def generate_enter_if_group_active(self):
        yield f'''\
do {self.ln.local_i} = 1, {self.ipn.count}
    call scorep_mpi_request_start( {self.ipn.array_of_requests}({self.ln.local_i}) )
end do
'''


class TaskRequestGetStatus(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request', 'flag', 'status')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_scorep_req',
                    type_name='type',
                    type_subtype='c_ptr'))
        )

    def generate_post_pmpi_call(self):
        yield f'''\
{self.ln.local_scorep_req} = scorep_mpi_request_get( {self.ipn.request} )
if ( {self.ipn.flag} ) then
    call scorep_mpi_check_request( {self.ln.local_scorep_req}, {self.ipn.status} )
    call scorep_mpi_request_set_completed( {self.ln.local_scorep_req} )
else if ( {self.ln.event_gen_active_for_group} .and. {self.ln.xreqtest_enabled} ) then
    call scorep_mpi_request_tested( {self.ln.local_scorep_req} )
end if
call scorep_mpi_unmark_request( {self.ln.local_scorep_req} )
'''


class TaskCancel(F08Task):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_scorep_req',
                    type_name='type',
                    type_subtype='c_ptr'))
        )

    def generate_pre_pmpi_call(self):
        yield f'''\
{self.ln.local_scorep_req} = scorep_mpi_request_get( {self.ipn.request} )
call scorep_mpi_request_set_cancel( {self.ln.local_scorep_req} )
call scorep_mpi_unmark_request( {self.ln.local_scorep_req} )
'''


class TaskRequestFree(F08Task):
    @staticmethod
    def has_conflict_with(task_type: Type['F08Task']) -> bool:
        # Ensure that this task is the first that provides the 'generate_post_pmpi_cal' hook
        return hasattr(task_type, 'generate_post_pmpi_call')

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def needs_local_variables(self) -> Scope[FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_got_null_request',
                    type_name='logical'),
                initial_value=f'({self.ipn.request} .eq. MPI_REQUEST_NULL)')
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_REQUEST_NULL'}

    def generate_pre_pmpi_call(self):
        yield f'call scorep_mpi_request_free_wrapper({self.ipn.request})\n'
        yield f'if ( {self.ipn.request} .ne. MPI_REQUEST_NULL .or. {self.ln.local_got_null_request} ) then \n'

    def generate_post_pmpi_call(self):
        yield 'end if\n'
