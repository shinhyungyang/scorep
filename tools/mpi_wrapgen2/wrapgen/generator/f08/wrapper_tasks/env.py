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
__all__ = ['TaskUseEnvGlobals',
           'TaskCommDestroyWorldSelf',
           'TaskCommCreateWorldSelf',
           'TaskInitialize',
           'TaskFinalize',
           'TaskCheckFinalized',
           'TaskDisableEventsOutsideMeasurement']

from typing import Set, Type, Dict

from wrapgen.language.fortran import FortranArgument, FortranVariable
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.f08 import scorep_f08_macros
from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope
from wrapgen.generator.utils_f import generate_multiline_comment


class TaskUseEnvGlobals(AuxiliaryTaskMixin, F08Task):
    def generate_use(self) -> GeneratorOutput:
        yield 'use :: SCOREP_MPI_Env_globals, only : mpi_finalize_called\n'


class TaskCommCreateWorldSelf(AuxiliaryTaskMixin, F08Task):
    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_COMM_WORLD', 'MPI_COMM_SELF'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_MpiCollectiveBegin()\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_CommCreate( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD ) )
call SCOREP_CommCreate( SCOREP_MPI_COMM_HANDLE( MPI_COMM_SELF ) )
call SCOREP_MpiCollectiveEnd( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD ), &
                              SCOREP_INVALID_ROOT_RANK, &
                              SCOREP_COLLECTIVE_CREATE_HANDLE, &
                              0_c_int64_t, &
                              0_c_int64_t )
'''


class TaskCommDestroyWorldSelf(AuxiliaryTaskMixin, F08Task):
    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_COMM_WORLD', 'MPI_COMM_SELF'}

    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'call SCOREP_MpiCollectiveBegin()\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield f'''\
call SCOREP_CommDestroy( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD) )
call SCOREP_CommDestroy( SCOREP_MPI_COMM_HANDLE( MPI_COMM_SELF) )
call SCOREP_MpiCollectiveEnd( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD), &
                              SCOREP_INVALID_ROOT_RANK, &
                              SCOREP_COLLECTIVE_DESTROY_HANDLE, &
                              0_c_int64_t, &
                              0_c_int64_t )
'''


class TaskInitialize(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskUseEnvGlobals, TaskCommCreateWorldSelf}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('ierror')

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='fflag',
                    type_name='logical'),
                initial_value='.false.'
            ),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='iflag',
                    type_name='logical'),
                initial_value='.false.'
            ),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_ierror',
                    type_name='integer')
            )
        )

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS', 'MPI_THREAD_FUNNELED', 'PMPI_Initialized', 'PMPI_Finalized'}

    def generate_initialization(self) -> GeneratorOutput:
        yield f'''\
mpi_finalize_called =.false.
if {scorep_f08_macros.SCOREP_IS_MEASUREMENT_PHASE("PRE")}  then
call SCOREP_InitMeasurement()
end if
'''

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'if ({self.ipn.ierror} .eq. MPI_SUCCESS) then\n'
        if 'required' in self.scp_procedure.scp_parameters and 'provided' in self.scp_procedure.scp_parameters:
            yield f'''\
if ((required .gt. MPI_THREAD_FUNNELED) .and. (provided .gt. MPI_THREAD_FUNNELED)) then
    {scorep_f08_macros.UTILS_WARNING('MPI environment initialization request and provided level exceed MPI_THREAD_FUNNELED!')}
end if
if ((required .eq. MPI_THREAD_FUNNELED) .and. (provided .ge. MPI_THREAD_FUNNELED) &
.and. (SCOREP_Location_GetId(SCOREP_Location_GetCurrentCPULocation()) .ne. 0)) then
    {scorep_f08_macros.UTILS_WARNING('MPI environment initialization with MPI_THREAD_FUNNELED not on master thread!')}
end if
'''
        yield f'''\
call PMPI_Initialized({self.ln.iflag}, {self.ln.local_ierror})
if ( {self.ln.iflag} .and. {self.ln.local_ierror} .eq. MPI_SUCCESS ) then
    call PMPI_Finalized( {self.ln.fflag}, {self.ln.local_ierror} )
    if ( .not. {self.ln.fflag} .and. {self.ln.local_ierror} .eq. MPI_SUCCESS ) then
        call SCOREP_InitMppMeasurement()
    end if
end if
'''
        yield 'end if\n'


class TaskFinalize(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskUseEnvGlobals, TaskCommDestroyWorldSelf}

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield from generate_multiline_comment('Be so kind and name the MPI_COMM_WORLD communicator\n',
                                              'if the user didn\'t do so already\n')
        yield 'call scorep_mpi_comm_set_default_names()\n'
        yield from generate_multiline_comment(
            'Finalize event handling\n',
            'We need to make sure that our exit handler is called before the MPI handler\n')
        yield 'call SCOREP_RegisterExitHandler()\n'


class TaskCheckFinalized(F08Task):
    @staticmethod
    def dependencies() -> Set[Type['F08Task']]:
        return {TaskUseEnvGlobals}

    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('flag', 'ierror')

    def needs_mpi_f08_uses(self) -> set[str]:
        return {'MPI_SUCCESS'}

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( {self.ipn.ierror} .eq. MPI_SUCCESS .and. mpi_finalize_called ) then
    {self.ipn.flag} = .true.
end if
'''


class TaskDisableEventsOutsideMeasurement(F08Task):
    def generate_comment(self) -> GeneratorOutput:
        yield '!> Events are only recorded inside the measurement phase.\n'

    def generate_initialization(self) -> GeneratorOutput:
        yield f'{self.ln.event_gen_active} = {self.ln.event_gen_active} .and. {scorep_f08_macros.SCOREP_IS_MEASUREMENT_PHASE("WITHIN")}\n'
