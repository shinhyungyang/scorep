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
from typing import Iterable

from wrapgen.generator.wrapper_hooks import WrapperHooks, GeneratorOutput
from wrapgen.generator.wrapper import Wrapper
from wrapgen.generator.utils_c import generate_wrapper_guard, generate_multiline_comment
from wrapgen.language.iso_c import CParameter, CVariable
from wrapgen.generator.scope import Scope

from wrapgen.generator.c.scorep_c_procedure import ScorepCProcedure


class CWrapperSkeleton(Wrapper):
    def __init__(self, scp_procedure: ScorepCProcedure, guards: Iterable[str]):
        super(Wrapper, self).__init__()
        self._scp_procedure = scp_procedure
        self._guards = guards

    @property
    def scp_procedure(self) -> ScorepCProcedure:
        return self._scp_procedure

    def generate(self) -> GeneratorOutput:
        yield from self.generate_guard_start()
        yield from self.generate_pmpi_declaration()
        # noinspection PyArgumentList
        yield from self.generate_comment()
        yield from self.generate_procedure_start()
        yield 'SCOREP_IN_MEASUREMENT_INCREMENT();\n'
        yield from self.generate_internal_declarations()
        yield from self.generate_local_declarations()
        # noinspection PyArgumentList
        yield from self.generate_initialization()
        # noinspection PyArgumentList
        yield from self.generate_body()
        # noinspection PyArgumentList
        yield from self.generate_cleanup()
        yield 'SCOREP_IN_MEASUREMENT_DECREMENT();\n'
        yield 'return return_value;\n'
        yield from self.generate_procedure_end()
        yield from self.generate_guard_end()
        yield '\n'

    def generate_guard_start(self) -> GeneratorOutput:
        yield from generate_wrapper_guard(self.scp_procedure.std_name,
                                          self.scp_procedure.version,
                                          self.scp_procedure.std_name,
                                          self._guards)

    def generate_pmpi_declaration(self) -> GeneratorOutput:
        yield self._procedure_decl(profiling=True)
        yield ';\n'

    def _generate_comment_post(self) -> GeneratorOutput:
        yield '*/\n'

    def _generate_comment_subclasses(self) -> GeneratorOutput:
        yield from ()

    @WrapperHooks.hook(post=_generate_comment_post)
    def generate_comment(self) -> GeneratorOutput:
        """
        The comment before the wrapper.
        """
        yield f"""\
/**
 * Measurement wrapper for {self.scp_procedure.std_name} in the C bindings.
 * @note Introduced in MPI {self.scp_procedure.version}
"""
        if self.scp_procedure.is_embiggened():
            yield '* @note Embiggened procedure\n'
        yield from self._generate_comment_subclasses()

    def generate_procedure_start(self) -> GeneratorOutput:
        yield self._procedure_decl(profiling=False)
        yield ' {\n'

    def generate_internal_declarations(self) -> GeneratorOutput:
        yield '/* Internal replacements for function arguments */\n'
        for p in self.scp_procedure.internal_parameters():
            yield f'{p};\n'

    def local_variables(self) -> Scope[CVariable]:
        return_type = self.scp_procedure.return_type
        assert return_type is not None, f'Function without return type in {self.scp_procedure.canonical_name}:' \
                                        f' {self.scp_procedure.std_name}'
        return Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='return_value', base_type=return_type)
            )
        )

    def generate_local_declarations(self) -> GeneratorOutput:
        yield '/* Local variable declarations */\n'
        all_locals = self.all_local_variables()
        for local_param in all_locals.values():
            yield f'{local_param}\n'
        yield '\n'

    @WrapperHooks.hook()
    def generate_initialization(self) -> GeneratorOutput:
        """
        Perform additional initialization, after local variables have
        been initialized.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_body(self) -> GeneratorOutput:
        """
        Wrapper body. After initialization.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_cleanup(self) -> GeneratorOutput:
        """
        Cleanup after the body of the wrapper.
        """
        yield from ()

    def generate_procedure_end(self) -> GeneratorOutput:
        yield '}\n'

    def generate_guard_end(self) -> GeneratorOutput:
        yield '#endif\n'

    def _procedure_decl(self, profiling=False) -> str:
        return_type = self.scp_procedure.return_type
        prefix = 'P' if profiling else ''
        name = f'{prefix}{self.scp_procedure.std_name}'
        if len(self.scp_procedure.scp_parameters) == 0:
            args = 'void'
        else:
            args = ', '.join([p.parameter.decl() for p in self.scp_procedure.scp_parameters.values()])
        return f'{return_type} {name}({args})'


class DefaultWrapper(CWrapperSkeleton):
    def __init__(self, scp_procedure: ScorepCProcedure, guards: Iterable[str], group: str):
        """
        :param scp_procedure:
        :param guards: Additional conditions for compile-time disabling of the
                       wrapper.
        :param group:  The Score-P function group this wrapper belongs to.
        """
        super().__init__(scp_procedure, guards)
        self._group = group

    def local_variables(self) -> Scope[CVariable]:
        lv = super(DefaultWrapper, self).local_variables()
        own = Scope.from_named_items(
            CVariable(
                parameter=CParameter(name='regionHandle', base_type='int', const=True),
                initial_value=f'scorep_mpi_regions[ SCOREP_MPI_REGION__{self.scp_procedure.canonical_name.upper()} ]'
            ),
            CVariable(
                parameter=CParameter(name='event_gen_active', base_type='int', const=False),
                initial_value='SCOREP_MPI_IS_EVENT_GEN_ON'
            ),
            CVariable(
                parameter=CParameter(name='event_gen_active_for_group', base_type='int', const=True),
                initial_value=f'SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_{self._group.upper()} )'
            )
        )
        lv.update(own)
        return lv

    def _generate_comment_subclasses(self) -> GeneratorOutput:
        yield f'* @ingroup {self._group}\n'

    @WrapperHooks.hook()
    def generate_initialization(self) -> GeneratorOutput:
        """
        Perform additional initialization after local variables have been
        initialized.

        Generates initialization for the parameters in the :class:`ScorepCProcedure`.
        """
        for p in self.scp_procedure.scp_parameters.values():
            yield from p.generate_init()

    @WrapperHooks.hook()
    def generate_body(self) -> GeneratorOutput:
        """
        Wrapper body. After initialization.
        """
        # noinspection PyArgumentList
        yield from self.generate_wrapper_enter()
        # noinspection PyArgumentList
        yield from self.generate_pre_pmpi_call()
        # noinspection PyArgumentList
        yield from self.generate_pmpi_call()
        # noinspection PyArgumentList
        yield from self.generate_post_pmpi_call()
        # noinspection PyArgumentList
        yield from self.generate_wrapper_exit()

    @WrapperHooks.hook()
    def generate_wrapper_enter(self) -> GeneratorOutput:
        """
        Block for recording ENTER events.
        """
        yield 'if (event_gen_active) {\n'
        # noinspection PyArgumentList
        yield from self.generate_enter_if_active()
        yield (
            'SCOREP_MPI_EVENT_GEN_OFF();\n'
            'if (event_gen_active_for_group) {\n'
        )
        # noinspection PyArgumentList
        yield from self.generate_enter_if_group_active_pre()
        yield f'SCOREP_EnterWrappedRegion(regionHandle);\n'
        # noinspection PyArgumentList
        yield from self.generate_enter_if_group_active()
        yield (
            '} else if (SCOREP_IsUnwindingEnabled()) {\n'
            'SCOREP_EnterWrapper(regionHandle);\n')
        # noinspection PyArgumentList
        yield from self.generate_enter_if_unwinding()
        yield (
            '}\n'
            '}\n'
            '\n')

    @WrapperHooks.hook()
    def generate_enter_if_active(self) -> GeneratorOutput:
        """
        Inside the branch if events are enabled in the
        :meth:`generate_wrapper_enter` block.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_enter_if_group_active(self) -> GeneratorOutput:
        """
        Inside the branch if events are enabled for the function
        group in the :meth:`generate_wrapper_enter` block.

        After the ENTER event has been written.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_enter_if_group_active_pre(self) -> GeneratorOutput:
        """
        Inside the branch if events are enabled for the function
        group in the :meth:`generate_wrapper_enter` block.

        Before the ENTER event has been written.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_enter_if_unwinding(self) -> GeneratorOutput:
        """
        Inside the branch if unwinding is enabled.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        """
        After the :meth:`generate_wrapper_enter` block, before the PMPI call.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_pmpi_call(self) -> GeneratorOutput:
        """
        The call to the PMPI function.
        """
        pmpi_name = f'P{self.scp_procedure.std_name}'
        return_value = self.local_variables()['return_value'].parameter.name
        pmpi_args = ', '.join([p.internal_parameter().name for p in self.scp_procedure.scp_parameters.values()])
        yield f'''\
SCOREP_ENTER_WRAPPED_REGION();
{return_value} = {pmpi_name}( {pmpi_args} );
SCOREP_EXIT_WRAPPED_REGION();

'''

    @WrapperHooks.hook()
    def generate_post_pmpi_call(self) -> GeneratorOutput:
        """
        After the call to the PMPI function, before the :meth:`generate_wrapper_exit`
        block.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_wrapper_exit(self) -> GeneratorOutput:
        """
        Block for recording EXIT events.
        """
        yield 'if (event_gen_active) {\n'
        # noinspection PyArgumentList
        yield from self.generate_exit_if_active()
        yield 'if (event_gen_active_for_group) {\n'
        # noinspection PyArgumentList
        yield from self.generate_exit_if_group_active()
        yield (
            'SCOREP_ExitRegion(regionHandle);\n'
            '} else if (SCOREP_IsUnwindingEnabled()) {\n')
        # noinspection PyArgumentList
        yield from self.generate_exit_if_unwinding()
        yield (
            'SCOREP_ExitWrapper(regionHandle);\n'
            '}\n'
            'SCOREP_MPI_EVENT_GEN_ON();\n'
            '}\n'
            '\n')

    @WrapperHooks.hook()
    def generate_exit_if_active(self) -> GeneratorOutput:
        """
        Inside the branch if events are enabled in the
        :meth:`generate_wrapper_exit` block.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_exit_if_group_active(self) -> GeneratorOutput:
        """
        Inside the branch if events are enabled for the function
        group in the :meth:`generate_wrapper_exit` block.

        Before the EXIT event has been written.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_exit_if_unwinding(self) -> GeneratorOutput:
        """
        Inside the branch if unwinding is enabled  in the
        :meth:`generate_wrapper_exit` block.
        """
        yield from ()

    @WrapperHooks.hook()
    def generate_cleanup(self) -> GeneratorOutput:
        """
        Generates cleanup code for the parameters in the :class:`ScorepCProcedure`.
        """
        for p in self.scp_procedure.scp_parameters.values():
            yield from p.generate_cleanup()


class FakeFinalizationWrapper(DefaultWrapper):
    """
    Special case for MPI_Finalize.
    """

    @WrapperHooks.hook()
    def generate_pmpi_call(self) -> GeneratorOutput:
        """
        Fake MPI finaliziation by calling PMPI_Barrier instead of PMPI_Finalize
        because Score-P needs to use MPI after the application is done.
        """
        return_value = self.local_variables()['return_value'].parameter.name
        yield 'SCOREP_ENTER_WRAPPED_REGION();\n'
        yield from generate_multiline_comment('Fake finalization\n')
        yield f'''\
{return_value} = PMPI_Barrier(MPI_COMM_WORLD);
SCOREP_EXIT_WRAPPED_REGION();
if (MPI_SUCCESS == {return_value})
{{
    mpi_finalize_called = true;
}}
'''


class GuardComplianceWrapper(DefaultWrapper):
    """
    Special case to disable procedures at compile time based on a configure time
    compliance check. The compliance check is necessary if an MPI implementation
    deviates from the standard.
    """

    def generate_guard_start(self) -> GeneratorOutput:
        yield f'#if HAVE( {self.scp_procedure.std_name.upper()}_COMPLIANT )\n'
        yield from super().generate_guard_start()

    def generate_guard_end(self) -> GeneratorOutput:
        yield from super().generate_guard_end()
        yield f'#endif\n'


class ProtoArgsWrapper(DefaultWrapper):
    """
    Special case to modify the procedure signature of a wrapper based on a
    configure check. This is necessary if an MPI implementation declares a
    procedure with a different signature than in the standard.
    """

    def proto_args(self):
        return f'SCOREP_{self.scp_procedure.std_name.upper()}_PROTO_ARGS'

    def generate_guard_start(self) -> GeneratorOutput:
        yield f'#if defined({self.proto_args()})\n'
        yield from super().generate_guard_start()

    def generate_pmpi_declaration(self) -> GeneratorOutput:
        yield f'{self.scp_procedure.return_type} P{self.scp_procedure.std_name} {self.proto_args()};\n'

    def generate_procedure_start(self) -> GeneratorOutput:
        yield f'{self.scp_procedure.return_type} {self.scp_procedure.std_name} {self.proto_args()}\n'
        yield '{\n'

    def generate_guard_end(self) -> GeneratorOutput:
        yield from super().generate_guard_end()
        yield '#endif\n'
