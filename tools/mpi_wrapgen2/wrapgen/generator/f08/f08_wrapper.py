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
import re
from typing import Iterable, cast

from wrapgen.generator.wrapper_hooks import WrapperHooks, GeneratorOutput
from wrapgen.generator.wrapper import Wrapper
from wrapgen.generator.scope import Scope
from wrapgen.generator.utils_f import generate_multiline_comment
from wrapgen.language.fortran import FortranArgument, FortranVariable

from .scorep_f08_procedure import ScorepF08Procedure
from . import scorep_f08_macros
from wrapgen.data.names import NamespaceDict


class FortranNamespaceDict(NamespaceDict):
    def __init__(self, _dict=None, /, **kwargs):
        super().__init__(str.lower, _dict, **kwargs)

    def update_with_list(self, l: list):
        self.update(FortranNamespaceDict({i: i for i in l}))


class F08WrapperSkeleton(Wrapper):
    def __init__(self, scp_procedure: ScorepF08Procedure, guards: Iterable[str]):
        super(Wrapper, self).__init__()
        self._scp_procedure = scp_procedure
        self._guards = guards

    @property
    def scp_procedure(self) -> ScorepF08Procedure:
        return self._scp_procedure

    def generate(self) -> GeneratorOutput:
        # noinspection PyArgumentList
        yield from self.generate_comment()
        yield from self.generate_guard_start()
        yield from self.generate_procedure_start()
        # noinspection PyArgumentList
        yield from self.generate_use()
        yield '\nimplicit none\n\n'
        yield from self.generate_declarations()
        yield from self.generate_local_declarations()
        yield 'call scorep_in_measurement_increment()\n'
        # noinspection PyArgumentList
        yield from self.generate_initialization()
        # noinspection PyArgumentList
        yield from self.generate_body()
        # noinspection PyArgumentList
        yield from self.generate_cleanup()
        yield 'call scorep_in_measurement_decrement()\n'
        yield from self.generate_procedure_end()
        yield from self.generate_guard_end()
        yield '\n'

    def _generate_comment_post(self) -> GeneratorOutput:
        yield '!>\n'

    def generate_comment_subclasses(self) -> GeneratorOutput:
        yield from ()

    @WrapperHooks.hook(post=_generate_comment_post)
    def generate_comment(self) -> GeneratorOutput:
        """
        The comment before the wrapper.
        """
        yield f'''!>
        !> Measurement wrapper for {self.scp_procedure.std_name} in the Fortran 2008 bindings.
        !> @note Introduced in MPI {self.scp_procedure.version}
        '''
        if self.scp_procedure.is_embiggened():
            yield '!> @note Embiggened procedure\n'
        yield from self.generate_comment_subclasses()

    def generate_guard_start(self) -> GeneratorOutput:
        yield f'#if defined ({self.scp_procedure.symbol_macro()})'
        for guard in self._guards:
            yield f' && !defined(SCOREP_MPI_NO_{guard.upper()})'
        yield '\n'

        yield f'''
#if {scorep_f08_macros.HAVE(self.scp_procedure.choice_buffers_macro())}
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
'''

    def generate_procedure_start(self) -> GeneratorOutput:
        yield f'subroutine {self.scp_procedure.symbol_macro()} (&\n'
        yield ', &\n'.join(self.scp_procedure.scp_parameters.keys())
        yield ')\n'
        yield '\n'

    @WrapperHooks.hook()
    def generate_use(self) -> GeneratorOutput:
        """
        Include additional Fortran modules by use statements.  These must be
        right at the beginning of the procedure.
        """
        yield 'use :: scorep_mpi_f08\n'
        yield f"use :: mpi_f08, only: &\n"
        yield ", &\n".join(sorted(self.mpi_f08_uses().values()))
        yield "\n"

    def generate_declarations(self) -> GeneratorOutput:
        yield '! Dummy argument declarations\n'
        for p in self.scp_procedure.dummy_parameters():
            yield f'{p}\n'
        yield '! Internal replacements for dummy arguments\n'
        for p in self.scp_procedure.internal_parameters():
            yield f'{p}\n'

    def local_variables(self) -> Scope[FortranVariable]:
        return Scope()

    def generate_local_declarations(self) -> GeneratorOutput:
        yield '! Local variable declarations\n'
        for lv in self.all_local_variables().values():
            yield f'{lv.parameter}\n'
        yield '\n'

    def mpi_f08_uses(self) -> FortranNamespaceDict:
        def _find_mpi_names_in_str(s: str):
            return re.findall(r"\bP?MPI_\w+", s, re.IGNORECASE)

        uses = FortranNamespaceDict()
        for p in self.scp_procedure.dummy_parameters():
            uses.update_with_list(_find_mpi_names_in_str(p.__str__()))
        for p in self.scp_procedure.internal_parameters():
            uses.update_with_list(_find_mpi_names_in_str(p.__str__()))
        for v in self.all_local_variables().values():
            uses.update_with_list(_find_mpi_names_in_str(cast(FortranVariable, v).__str__()))
            uses.update_with_list(_find_mpi_names_in_str(cast(FortranVariable, v).initialization()))
        for task in self.attached_tasks():
            uses.update_with_list(list(task.needs_mpi_f08_uses()))
        return uses

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
        yield 'end subroutine\n'

    def generate_guard_end(self) -> GeneratorOutput:
        yield '#undef CHOICE_BUFFER_TYPE\n'
        yield '#endif\n'


class DefaultWrapper(F08WrapperSkeleton):
    def __init__(self, scp_procedure: ScorepF08Procedure, guards: Iterable[str], group: str):
        """
        :param scp_procedure:
        :param guards: Additional conditions for compile-time disabling of the
                       wrapper.
        :param group:  The Score-P function group this wrapper belongs to.
        """
        super().__init__(scp_procedure, guards)
        self._group = group

    def local_variables(self) -> Scope[FortranVariable]:
        lv = super(DefaultWrapper, self).local_variables()
        lv.update(Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='regionHandle',
                    type_name='integer',
                    type_parameters={'kind': 'SCOREP_InterimCommunicatorHandle'}),
                initial_value=f'scorep_mpi_regions(SCOREP_MPI_REGION__{self.scp_procedure.canonical_name.upper()})'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='event_gen_active',
                    type_name='logical'
                ),
                initial_value=f'{scorep_f08_macros.SCOREP_MPI_IS_EVENT_GEN_ON()}'
            ),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='event_gen_active_for_group',
                    type_name='logical'
                ),
                initial_value=f'{scorep_f08_macros.SCOREP_MPI_IS_EVENT_GEN_ON_FOR(self._group)}'
            ),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='scorep_in_measurement_save',
                    type_name='integer',
                    type_parameters={'kind': 'c_int'}))
        ))
        return lv

    def mpi_f08_uses(self) -> FortranNamespaceDict:
        uses = super().mpi_f08_uses()
        uses.update_with_list([f'P{self.scp_procedure.std_name}'])
        return uses

    def generate_comment_subclasses(self) -> GeneratorOutput:
        yield f'!> @ingroup {self._group}\n'

    @WrapperHooks.hook()
    def generate_initialization(self) -> GeneratorOutput:
        """
        Perform additional initialization after local variables have been
        initialized.

        Generates initialization for the parameters in the :class:`ScorepF08Procedure`.
        """
        for lv in self.all_local_variables().values():
            if lv.initial_value != '':
                yield f'{lv.initialization()}\n'

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
        yield 'if (event_gen_active) then\n'
        # noinspection PyArgumentList
        yield from self.generate_enter_if_active()
        yield from (
            scorep_f08_macros.SCOREP_MPI_EVENT_GEN_OFF(), '\n',
            'if (event_gen_active_for_group) then\n'
        )
        # noinspection PyArgumentList
        yield from self.generate_enter_if_group_active_pre()
        yield 'call SCOREP_EnterWrappedRegion(regionHandle)\n'
        # noinspection PyArgumentList
        yield from self.generate_enter_if_group_active()
        yield from (
            f'else if {scorep_f08_macros.SCOREP_IsUnwindingEnabled()} then\n',
            'call SCOREP_EnterWrapper(regionHandle)\n',
        )
        # noinspection PyArgumentList
        yield from self.generate_enter_if_unwinding()
        yield from (
            'end if\n',
            'end if\n',
            '\n'
        )

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
        Inside the branch if unwinding is enabled in the :meth:`generate_wrapper_enter`
        block.
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
        yield f'call scorep_enter_wrapped_region( {self.local_variables().scorep_in_measurement_save.name} )\n'
        yield f'call {pmpi_name}(&\n'
        yield ',&\n'.join([p.internal_parameter().name for p in self.scp_procedure.scp_parameters.values()])
        yield ')\n'
        yield f'call scorep_exit_wrapped_region( {self.local_variables().scorep_in_measurement_save.name} )\n'
        yield '\n'

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
        yield 'if (event_gen_active) then\n'
        # noinspection PyArgumentList
        yield from self.generate_exit_if_active()
        yield 'if (event_gen_active_for_group) then\n'
        # noinspection PyArgumentList
        yield from self.generate_exit_if_group_active()
        yield from (
            'call SCOREP_ExitRegion(regionHandle)\n',
            f'else if {scorep_f08_macros.SCOREP_IsUnwindingEnabled()} then\n',
        )
        # noinspection PyArgumentList
        yield from self.generate_exit_if_unwinding()
        yield from (
            'call SCOREP_ExitWrapper(regionHandle)\n',
            'end if\n',
            f'{scorep_f08_macros.SCOREP_MPI_EVENT_GEN_ON()}\n',
            'end if\n',
            '\n'
        )

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
        Generates cleanup code for the parameters in the :class:`ScorepF08Procedure`.

        Handles optional intent(out) arguments.
        """
        for p in self.scp_procedure.scp_parameters.values():
            yield from p.generate_cleanup()


class FakeFinalizationWrapper(DefaultWrapper):
    """
    Special case for MPI_Finalize.
    """

    def mpi_f08_uses(self) -> FortranNamespaceDict:
        uses = super().mpi_f08_uses()
        uses.update_with_list(["PMPI_Barrier, MPI_COMM_WORLD, MPI_SUCCESS"])
        return uses

    @WrapperHooks.hook()
    def generate_pmpi_call(self) -> GeneratorOutput:
        """
        Fake MPI finaliziation by calling PMPI_Barrier instead of PMPI_Finalize
        because Score-P needs to use MPI after the application is done.
        """
        ierror = self.scp_procedure.scp_parameters['ierror'].internal_parameter().name
        yield f'call scorep_enter_wrapped_region( {self.local_variables().scorep_in_measurement_save.name} )\n'
        yield from generate_multiline_comment("Fake finalization\n")
        yield f'call PMPI_Barrier(MPI_COMM_WORLD, {ierror})\n'
        yield f'call scorep_exit_wrapped_region( {self.local_variables().scorep_in_measurement_save.name} )\n'
        yield from (
            f'if ({ierror} .eq. MPI_SUCCESS) then\n',
            'mpi_finalize_called = .true.\n'
            'end if\n'
        )


class GuardComplianceWrapper(DefaultWrapper):
    """
    Compliance checks are only meaningful for the C Layer.
    """
    pass


class ProtoArgsWrapper(DefaultWrapper):
    """
    Checks for non-standard function prototypes are only meaningful in the C Layer.
    """
    pass
