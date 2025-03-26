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
__all__ = ['TaskInitialize',
           'TaskFinalize',
           'TaskCheckFinalized',
           'TaskCommCreateWorldSelf',
           'TaskCommDestroyWorldSelf',
           'TaskDisableEventsOutsideMeasurement']

from typing import Set, Type

from wrapgen.language.iso_c import CVariable, CParameter
from wrapgen.generator import GeneratorOutput
from wrapgen.generator.c.wrapper_tasks import CTask
from wrapgen.generator.task import AuxiliaryTaskMixin
from wrapgen.generator.scope import Scope


class TaskCommCreateWorldSelf(AuxiliaryTaskMixin, CTask):
    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'SCOREP_MpiCollectiveBegin();\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield '''
SCOREP_CommCreate( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD ) );
SCOREP_CommCreate( SCOREP_MPI_COMM_HANDLE( MPI_COMM_SELF ) );
SCOREP_MpiCollectiveEnd( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD ),
                         SCOREP_INVALID_ROOT_RANK,
                         SCOREP_COLLECTIVE_CREATE_HANDLE,
                         0,
                         0 );
'''


class TaskCommDestroyWorldSelf(AuxiliaryTaskMixin, CTask):
    def generate_enter_if_group_active(self) -> GeneratorOutput:
        yield 'SCOREP_MpiCollectiveBegin();\n'

    def generate_exit_if_group_active(self) -> GeneratorOutput:
        yield '''\
SCOREP_CommDestroy( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD) );
SCOREP_CommDestroy( SCOREP_MPI_COMM_HANDLE( MPI_COMM_SELF) );
SCOREP_MpiCollectiveEnd( SCOREP_MPI_COMM_HANDLE( MPI_COMM_WORLD),
                         SCOREP_INVALID_ROOT_RANK,
                         SCOREP_COLLECTIVE_DESTROY_HANDLE,
                         0,
                         0 );
'''


class TaskInitialize(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskCommCreateWorldSelf}

    def needs_local_variables(self) -> Scope[CVariable]:
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='fflag', base_type='int'),
                initial_value='0'
            ),
            CVariable(
                parameter=CParameter(name='iflag', base_type='int'),
                initial_value='0'
            )
        )

    def generate_initialization(self) -> GeneratorOutput:
        yield '''\
if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
{
    /* Initialize the measurement system */
    SCOREP_InitMeasurement();
}
'''

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield 'if ( return_value == MPI_SUCCESS ) {\n'
        if 'required' in self.scp_procedure.scp_parameters and 'provided' in self.scp_procedure.scp_parameters:
            yield '''\
if ( ( required > MPI_THREAD_FUNNELED ) && ( *provided > MPI_THREAD_FUNNELED ) )
{
#if HAVE( THREAD_LOCAL_STORAGE )
    UTILS_WARNING( "Support for MPI_THREAD_(SERIALIZED|MULTIPLE) is currently experimental!\\n"
                   "Please report bugs and give feedback to support@score-p.org." );
#else
    /* If no thread-local storage specifier is availble we only support MPI_THREAD_SERIALIZED. */

    UTILS_WARNING( "No thread-local storage specifier detected during configure, therefore\\n"
                   "MPI_THREAD_MULTIPLE is not supported!\\n\\n"
                   "Support for MPI_THREAD_SERIALIZED is currently experimental!\\n"
                   "Please report bugs and give feedback to support@score-p.org." );
#endif /* ! HAVE( THREAD_LOCAL_STORAGE ) */
}
if ( ( required == MPI_THREAD_FUNNELED ) && ( *provided >= MPI_THREAD_FUNNELED )
     && 0 != SCOREP_Location_GetId( SCOREP_Location_GetCurrentCPULocation() ) )
{
    UTILS_WARNING( "MPI environment initialization with MPI_THREAD_FUNNELED not on master thread!" );
}
'''
        yield f'''
if ( ( PMPI_Initialized( &{self.ln.iflag} ) == MPI_SUCCESS ) && ( {self.ln.iflag} != 0 ) &&
     ( PMPI_Finalized( &{self.ln.fflag} ) == MPI_SUCCESS ) && ( {self.ln.fflag} == 0 ) )
{{
    /* Complete initialization of measurement core and MPI event handling */
    SCOREP_InitMppMeasurement();
}}
'''
        yield '}\n'


class TaskFinalize(CTask):
    @staticmethod
    def dependencies() -> Set[Type['CTask']]:
        return {TaskCommDestroyWorldSelf}

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield '''\
    /* Be so kind and name the MPI_COMM_WORLD, if the user didn't do so already */
    scorep_mpi_comm_set_default_names();

    /* finalize MPI event handling */
    /* We need to make sure that our exit handler is called before the MPI one. */
    SCOREP_RegisterExitHandler();
'''


class TaskCheckFinalized(CTask):
    def needs_parameter_names(self) -> Scope[str]:
        return self.make_parameter_name_mapping('flag')

    def generate_post_pmpi_call(self) -> GeneratorOutput:
        yield f'''\
if ( MPI_SUCCESS == return_value && mpi_finalize_called )
{{
    *{self.ipn.flag} = 1;
}}
'''


class TaskDisableEventsOutsideMeasurement(CTask):
    def generate_comment(self) -> GeneratorOutput:
        yield '* Events are only recorded inside the measurement phase.\n'

    def generate_initialization(self) -> GeneratorOutput:
        yield f'{self.ln.event_gen_active} &= SCOREP_IS_MEASUREMENT_PHASE( WITHIN );\n'
