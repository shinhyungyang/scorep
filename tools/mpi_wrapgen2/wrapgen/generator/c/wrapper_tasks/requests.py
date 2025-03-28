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
           'TaskTest',
           'TaskWaitall',
           'TaskWaitany',
           'TaskTestall',
           'TaskTestany',
           'TaskSaveRequestArray',
           'TaskTestWaitSome',
           'TaskStart',
           'TaskStartall',
           'TaskRequestGetStatus',
           'TaskXreqtestEnabled',
           'TaskCancel',
           'TaskRequestFree']

from typing import Set, Type

from wrapgen.language.iso_c import CVariable, CParameter
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskXreqtestEnabled(AuxiliaryTaskMixin, CTask):
    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='xreqtest_enabled',
                                           base_type='int',
                                           const=True),
                      initial_value='SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_XREQTEST )')
        )


class TaskWait(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('status')

    def generate_post_pmpi_call(self):
        yield f'scorep_mpi_check_all_or_none( 1, REQUESTS_COMPLETED, {self.ipn.status} );\n'


class TaskWaitall(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'array_of_statuses')

    def generate_post_pmpi_call(self):
        yield f'scorep_mpi_check_all_or_none( {self.ipn.count}, REQUESTS_COMPLETED, {self.ipn.array_of_statuses} );\n'


class TaskWaitany(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('index', 'count', 'status')

    def generate_post_pmpi_call(self):
        yield f'''\
/* array_of_requests contains active handles */
if ( *{self.ipn.index} != MPI_UNDEFINED )
{{
    if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
    {{
        scorep_mpi_check_some_test_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} );
    }}
    else
    {{
        scorep_mpi_check_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} );
    }}
}}
'''


class TaskTestWaitSome(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('incount', 'outcount', 'array_of_indices', 'array_of_statuses')

    def generate_post_pmpi_call(self):
        yield f'''\
/* array_of_requests contains active handles */
if ( *{self.ipn.outcount} != MPI_UNDEFINED )
{{
    if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
    {{
        scorep_mpi_check_some_test_some( {self.ipn.incount}, *{self.ipn.outcount}, {self.ipn.array_of_indices}, {self.ipn.array_of_statuses} );
    }}
    else
    {{
        scorep_mpi_check_some( {self.ipn.incount}, *{self.ipn.outcount}, {self.ipn.array_of_indices}, {self.ipn.array_of_statuses} );
    }}
}}
'''


class TaskTest(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('flag', 'status')

    def generate_post_pmpi_call(self):
        yield f'''\
if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
{{
    scorep_mpi_check_all_or_test_all( 1, *{self.ipn.flag}, {self.ipn.status} );
}}
else
{{
    scorep_mpi_check_all_or_none( 1, *{self.ipn.flag}, {self.ipn.status} );
}}
'''


class TaskTestall(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'flag', 'array_of_statuses')

    def generate_post_pmpi_call(self):
        yield f'''\
if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
{{
    scorep_mpi_check_all_or_test_all( {self.ipn.count}, *{self.ipn.flag}, {self.ipn.array_of_statuses} );
}}
else
{{
    scorep_mpi_check_all_or_none( {self.ipn.count}, *{self.ipn.flag}, {self.ipn.array_of_statuses} );
}}
'''


class TaskTestany(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskSaveRequestArray, TaskXreqtestEnabled}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'flag', 'status', 'index')

    def generate_post_pmpi_call(self):
        yield f'''\
 /* array_of_requests contains active handles, but none were completed */
if ( !*{self.ipn.flag} )
{{
    if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
    {{
        scorep_mpi_test_all( {self.ipn.count} );
    }}
}}
 /* array_of_requests contains active handles, and one was completed */
else if ( *{self.ipn.index} != MPI_UNDEFINED )
{{
    if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
    {{
        scorep_mpi_check_some_test_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} );
    }}
    else
    {{
        scorep_mpi_check_some( {self.ipn.count}, 1, {self.ipn.index}, {self.ipn.status} );
    }}
}}
/* else, array_of_requests contains no active handles */
'''


class TaskStart(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def generate_enter_if_group_active(self):
        yield f'scorep_mpi_request_start(*{self.ipn.request});\n'


class TaskStartall(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('count', 'array_of_requests')

    def generate_enter_if_group_active(self):
        yield f'''\
for(int i = 0; i < {self.ipn.count}; ++i)
{{
    scorep_mpi_request_start( {self.ipn.array_of_requests}[i] );
}}
'''


class TaskRequestGetStatus(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskXreqtestEnabled}

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_scorep_req', base_type='scorep_mpi_request', pointer_level=1))
        )

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request', 'flag', 'status')

    def generate_post_pmpi_call(self):
        yield f'''\
{self.ln.local_scorep_req} = scorep_mpi_request_get( {self.ipn.request} );
if ( *{self.ipn.flag} )
{{
    scorep_mpi_check_request( {self.ln.local_scorep_req}, {self.ipn.status} );
    scorep_mpi_request_set_completed( {self.ln.local_scorep_req} );
}}
else if ( {self.ln.event_gen_active_for_group} && {self.ln.xreqtest_enabled} )
{{
    scorep_mpi_request_tested( {self.ln.local_scorep_req} );
}}
scorep_mpi_unmark_request( {self.ln.local_scorep_req} );
'''


class TaskCancel(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_scorep_req', base_type='scorep_mpi_request', pointer_level=1))
        )

    def generate_pre_pmpi_call(self):
        yield f'''\
{self.ln.local_scorep_req} = scorep_mpi_request_get( *{self.ipn.request} );
scorep_mpi_request_set_cancel( {self.ln.local_scorep_req} );
scorep_mpi_unmark_request( {self.ln.local_scorep_req} );
'''


class TaskRequestFree(CTask):
    @staticmethod
    def has_conflict_with(task_type: Type['CTask']) -> bool:
        # Ensure that this task is the first that provides the 'generate_post_pmpi_cal' hook
        return hasattr(task_type, 'generate_post_pmpi_call')

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('request')

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(parameter=CParameter(name='local_got_null_request', base_type='int', const=True),
                      initial_value=f'(*{self.ipn.request} == MPI_REQUEST_NULL)')
        )

    def generate_pre_pmpi_call(self):
        yield f'scorep_mpi_request_free_wrapper( {self.ipn.request} );\n'
        yield f'if ( *{self.ipn.request} != MPI_REQUEST_NULL || {self.ln.local_got_null_request} ) \n'
        yield '{\n'

    def generate_post_pmpi_call(self):
        yield '}\n'


class TaskSaveRequestArray(AuxiliaryTaskMixin, CTask):
    def needs_parameter_names(self) -> Scope[str]:
        name_mapping = self.make_parameter_name_mapping(req=['request', 'array_of_requests'])
        if name_mapping.req == 'array_of_requests':
            name_mapping.update(self.make_parameter_name_mapping(count=['count', 'incount']))
        return name_mapping

    def generate_pre_pmpi_call(self):
        if 'count' in self.ipn:
            count = self.ipn.count
        else:
            count = '1'

        yield f'scorep_mpi_save_request_array( {self.ipn.req}, {count} ); \n'
