!
! This file is part of the Score-P software (http://www.score-p.org)
!
! Copyright (c) 2025,
! Forschungszentrum Juelich GmbH, Germany
!
! This software may be modified and distributed under the terms of
! a BSD-style license. See the COPYING file in the package base
! directory for details.
!

!
! This file was generated by mpi_wrapgen2 from template adapter/f08/wrappers/SCOREP_Mpi_F08_Err.F90.tmpl.
!

!>
!> @file
!> @ingroup    MPI_Wrapper
!>
!> @brief This file is currently a placeholder
!>
#include <config.h>

!>
!> @name F08 wrappers
!>
!> @{
!>

!>
!> Measurement wrapper for MPI_Add_error_class in the Fortran 2008 bindings.
!> @note Introduced in MPI 2.0
!> @ingroup err
!> Default wrapper: Only enter and exit events are created.
!>
#if defined (SCOREP_F08_SYMBOL_NAME_MPI_ADD_ERROR_CLASS) && !defined(SCOREP_MPI_NO_EXTRA) && !defined(SCOREP_MPI_NO_ERR)

#if ( defined( HAVE_F08_TS_BUFFERS_MPI_ADD_ERROR_CLASS ) && HAVE_F08_TS_BUFFERS_MPI_ADD_ERROR_CLASS )
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
subroutine SCOREP_F08_SYMBOL_NAME_MPI_ADD_ERROR_CLASS( &
    errorclass, &
    ierror)

    use :: scorep_mpi_f08
    use :: mpi_f08, only: &
        PMPI_Add_error_class

    implicit none

! Dummy argument declarations
    INTEGER, INTENT(OUT) :: errorclass
    INTEGER, INTENT(OUT), OPTIONAL :: ierror
! Internal replacements for dummy arguments
    INTEGER :: internal_ierror
! Local variable declarations
    integer(kind=SCOREP_InterimCommunicatorHandle) :: regionHandle
    logical :: event_gen_active
    logical :: event_gen_active_for_group
    integer(kind=c_int) :: scorep_in_measurement_save

    call scorep_in_measurement_increment()
    regionHandle = scorep_mpi_regions(SCOREP_MPI_REGION__MPI_ADD_ERROR_CLASS)
    event_gen_active = logical(scorep_mpi_is_event_gen_on())
    event_gen_active_for_group = (logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_ERR) .gt&
    &. 0))
    if (event_gen_active) then
        call scorep_mpi_event_gen_off()
        if (event_gen_active_for_group) then
            call SCOREP_EnterWrappedRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_EnterWrapper(regionHandle)
        end if
    end if

    call scorep_enter_wrapped_region(scorep_in_measurement_save)
    call PMPI_Add_error_class( &
        errorclass, &
        internal_ierror)
    call scorep_exit_wrapped_region(scorep_in_measurement_save)

    if (event_gen_active) then
        if (event_gen_active_for_group) then
            call SCOREP_ExitRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_ExitWrapper(regionHandle)
        end if
        call scorep_mpi_event_gen_on()
    end if

    if (present(ierror)) then
        ierror = internal_ierror
    end if
    call scorep_in_measurement_decrement()
end subroutine
#undef CHOICE_BUFFER_TYPE
#endif

!>
!> Measurement wrapper for MPI_Add_error_code in the Fortran 2008 bindings.
!> @note Introduced in MPI 2.0
!> @ingroup err
!> Default wrapper: Only enter and exit events are created.
!>
#if defined (SCOREP_F08_SYMBOL_NAME_MPI_ADD_ERROR_CODE) && !defined(SCOREP_MPI_NO_EXTRA) && !defined(SCOREP_MPI_NO_ERR)

#if ( defined( HAVE_F08_TS_BUFFERS_MPI_ADD_ERROR_CODE ) && HAVE_F08_TS_BUFFERS_MPI_ADD_ERROR_CODE )
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
subroutine SCOREP_F08_SYMBOL_NAME_MPI_ADD_ERROR_CODE( &
    errorclass, &
    errorcode, &
    ierror)

    use :: scorep_mpi_f08
    use :: mpi_f08, only: &
        PMPI_Add_error_code

    implicit none

! Dummy argument declarations
    INTEGER, INTENT(IN) :: errorclass
    INTEGER, INTENT(OUT) :: errorcode
    INTEGER, INTENT(OUT), OPTIONAL :: ierror
! Internal replacements for dummy arguments
    INTEGER :: internal_ierror
! Local variable declarations
    integer(kind=SCOREP_InterimCommunicatorHandle) :: regionHandle
    logical :: event_gen_active
    logical :: event_gen_active_for_group
    integer(kind=c_int) :: scorep_in_measurement_save

    call scorep_in_measurement_increment()
    regionHandle = scorep_mpi_regions(SCOREP_MPI_REGION__MPI_ADD_ERROR_CODE)
    event_gen_active = logical(scorep_mpi_is_event_gen_on())
    event_gen_active_for_group = (logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_ERR) .gt&
    &. 0))
    if (event_gen_active) then
        call scorep_mpi_event_gen_off()
        if (event_gen_active_for_group) then
            call SCOREP_EnterWrappedRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_EnterWrapper(regionHandle)
        end if
    end if

    call scorep_enter_wrapped_region(scorep_in_measurement_save)
    call PMPI_Add_error_code( &
        errorclass, &
        errorcode, &
        internal_ierror)
    call scorep_exit_wrapped_region(scorep_in_measurement_save)

    if (event_gen_active) then
        if (event_gen_active_for_group) then
            call SCOREP_ExitRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_ExitWrapper(regionHandle)
        end if
        call scorep_mpi_event_gen_on()
    end if

    if (present(ierror)) then
        ierror = internal_ierror
    end if
    call scorep_in_measurement_decrement()
end subroutine
#undef CHOICE_BUFFER_TYPE
#endif

!>
!> Measurement wrapper for MPI_Add_error_string in the Fortran 2008 bindings.
!> @note Introduced in MPI 2.0
!> @ingroup err
!> Default wrapper: Only enter and exit events are created.
!>
#if defined (SCOREP_F08_SYMBOL_NAME_MPI_ADD_ERROR_STRING) && !defined(SCOREP_MPI_NO_EXTRA) && !defined(SCOREP_MPI_NO_ERR)

#if ( defined( HAVE_F08_TS_BUFFERS_MPI_ADD_ERROR_STRING ) && HAVE_F08_TS_BUFFERS_MPI_ADD_ERROR_STRING )
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
subroutine SCOREP_F08_SYMBOL_NAME_MPI_ADD_ERROR_STRING( &
    errorcode, &
    string, &
    ierror)

    use :: scorep_mpi_f08
    use :: mpi_f08, only: &
        PMPI_Add_error_string

    implicit none

! Dummy argument declarations
    INTEGER, INTENT(IN) :: errorcode
    CHARACTER(LEN=*), INTENT(IN) :: string
    INTEGER, INTENT(OUT), OPTIONAL :: ierror
! Internal replacements for dummy arguments
    INTEGER :: internal_ierror
! Local variable declarations
    integer(kind=SCOREP_InterimCommunicatorHandle) :: regionHandle
    logical :: event_gen_active
    logical :: event_gen_active_for_group
    integer(kind=c_int) :: scorep_in_measurement_save

    call scorep_in_measurement_increment()
    regionHandle = scorep_mpi_regions(SCOREP_MPI_REGION__MPI_ADD_ERROR_STRING)
    event_gen_active = logical(scorep_mpi_is_event_gen_on())
    event_gen_active_for_group = (logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_ERR) .gt&
    &. 0))
    if (event_gen_active) then
        call scorep_mpi_event_gen_off()
        if (event_gen_active_for_group) then
            call SCOREP_EnterWrappedRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_EnterWrapper(regionHandle)
        end if
    end if

    call scorep_enter_wrapped_region(scorep_in_measurement_save)
    call PMPI_Add_error_string( &
        errorcode, &
        string, &
        internal_ierror)
    call scorep_exit_wrapped_region(scorep_in_measurement_save)

    if (event_gen_active) then
        if (event_gen_active_for_group) then
            call SCOREP_ExitRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_ExitWrapper(regionHandle)
        end if
        call scorep_mpi_event_gen_on()
    end if

    if (present(ierror)) then
        ierror = internal_ierror
    end if
    call scorep_in_measurement_decrement()
end subroutine
#undef CHOICE_BUFFER_TYPE
#endif

!>
!> Measurement wrapper for MPI_Errhandler_free in the Fortran 2008 bindings.
!> @note Introduced in MPI 1.0
!> @ingroup err
!> Default wrapper: Only enter and exit events are created.
!>
#if defined (SCOREP_F08_SYMBOL_NAME_MPI_ERRHANDLER_FREE) && !defined(SCOREP_MPI_NO_EXTRA) && !defined(SCOREP_MPI_NO_ERR)

#if ( defined( HAVE_F08_TS_BUFFERS_MPI_ERRHANDLER_FREE ) && HAVE_F08_TS_BUFFERS_MPI_ERRHANDLER_FREE )
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
subroutine SCOREP_F08_SYMBOL_NAME_MPI_ERRHANDLER_FREE( &
    errhandler, &
    ierror)

    use :: scorep_mpi_f08
    use :: mpi_f08, only: &
        MPI_ERRHANDLER, &
        PMPI_Errhandler_free

    implicit none

! Dummy argument declarations
    TYPE(MPI_ERRHANDLER), INTENT(INOUT) :: errhandler
    INTEGER, INTENT(OUT), OPTIONAL :: ierror
! Internal replacements for dummy arguments
    INTEGER :: internal_ierror
! Local variable declarations
    integer(kind=SCOREP_InterimCommunicatorHandle) :: regionHandle
    logical :: event_gen_active
    logical :: event_gen_active_for_group
    integer(kind=c_int) :: scorep_in_measurement_save

    call scorep_in_measurement_increment()
    regionHandle = scorep_mpi_regions(SCOREP_MPI_REGION__MPI_ERRHANDLER_FREE)
    event_gen_active = logical(scorep_mpi_is_event_gen_on())
    event_gen_active_for_group = (logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_ERR) .gt&
    &. 0))
    if (event_gen_active) then
        call scorep_mpi_event_gen_off()
        if (event_gen_active_for_group) then
            call SCOREP_EnterWrappedRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_EnterWrapper(regionHandle)
        end if
    end if

    call scorep_enter_wrapped_region(scorep_in_measurement_save)
    call PMPI_Errhandler_free( &
        errhandler, &
        internal_ierror)
    call scorep_exit_wrapped_region(scorep_in_measurement_save)

    if (event_gen_active) then
        if (event_gen_active_for_group) then
            call SCOREP_ExitRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_ExitWrapper(regionHandle)
        end if
        call scorep_mpi_event_gen_on()
    end if

    if (present(ierror)) then
        ierror = internal_ierror
    end if
    call scorep_in_measurement_decrement()
end subroutine
#undef CHOICE_BUFFER_TYPE
#endif

!>
!> Measurement wrapper for MPI_Error_class in the Fortran 2008 bindings.
!> @note Introduced in MPI 1.0
!> @ingroup err
!> Default wrapper: Only enter and exit events are created.
!>
#if defined (SCOREP_F08_SYMBOL_NAME_MPI_ERROR_CLASS) && !defined(SCOREP_MPI_NO_EXTRA) && !defined(SCOREP_MPI_NO_ERR)

#if ( defined( HAVE_F08_TS_BUFFERS_MPI_ERROR_CLASS ) && HAVE_F08_TS_BUFFERS_MPI_ERROR_CLASS )
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
subroutine SCOREP_F08_SYMBOL_NAME_MPI_ERROR_CLASS( &
    errorcode, &
    errorclass, &
    ierror)

    use :: scorep_mpi_f08
    use :: mpi_f08, only: &
        PMPI_Error_class

    implicit none

! Dummy argument declarations
    INTEGER, INTENT(IN) :: errorcode
    INTEGER, INTENT(OUT) :: errorclass
    INTEGER, INTENT(OUT), OPTIONAL :: ierror
! Internal replacements for dummy arguments
    INTEGER :: internal_ierror
! Local variable declarations
    integer(kind=SCOREP_InterimCommunicatorHandle) :: regionHandle
    logical :: event_gen_active
    logical :: event_gen_active_for_group
    integer(kind=c_int) :: scorep_in_measurement_save

    call scorep_in_measurement_increment()
    regionHandle = scorep_mpi_regions(SCOREP_MPI_REGION__MPI_ERROR_CLASS)
    event_gen_active = logical(scorep_mpi_is_event_gen_on())
    event_gen_active_for_group = (logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_ERR) .gt&
    &. 0))
    if (event_gen_active) then
        call scorep_mpi_event_gen_off()
        if (event_gen_active_for_group) then
            call SCOREP_EnterWrappedRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_EnterWrapper(regionHandle)
        end if
    end if

    call scorep_enter_wrapped_region(scorep_in_measurement_save)
    call PMPI_Error_class( &
        errorcode, &
        errorclass, &
        internal_ierror)
    call scorep_exit_wrapped_region(scorep_in_measurement_save)

    if (event_gen_active) then
        if (event_gen_active_for_group) then
            call SCOREP_ExitRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_ExitWrapper(regionHandle)
        end if
        call scorep_mpi_event_gen_on()
    end if

    if (present(ierror)) then
        ierror = internal_ierror
    end if
    call scorep_in_measurement_decrement()
end subroutine
#undef CHOICE_BUFFER_TYPE
#endif

!>
!> Measurement wrapper for MPI_Error_string in the Fortran 2008 bindings.
!> @note Introduced in MPI 1.0
!> @ingroup err
!> Default wrapper: Only enter and exit events are created.
!>
#if defined (SCOREP_F08_SYMBOL_NAME_MPI_ERROR_STRING) && !defined(SCOREP_MPI_NO_EXTRA) && !defined(SCOREP_MPI_NO_ERR)

#if ( defined( HAVE_F08_TS_BUFFERS_MPI_ERROR_STRING ) && HAVE_F08_TS_BUFFERS_MPI_ERROR_STRING )
#define CHOICE_BUFFER_TYPE type(*), dimension(..)
#else
#define CHOICE_BUFFER_TYPE type(*), dimension(*)
#endif
subroutine SCOREP_F08_SYMBOL_NAME_MPI_ERROR_STRING( &
    errorcode, &
    string, &
    resultlen, &
    ierror)

    use :: scorep_mpi_f08
    use :: mpi_f08, only: &
        MPI_MAX_ERROR_STRING, &
        PMPI_Error_string

    implicit none

! Dummy argument declarations
    INTEGER, INTENT(IN) :: errorcode
    CHARACTER(LEN=MPI_MAX_ERROR_STRING), INTENT(OUT) :: string
    INTEGER, INTENT(OUT) :: resultlen
    INTEGER, INTENT(OUT), OPTIONAL :: ierror
! Internal replacements for dummy arguments
    INTEGER :: internal_ierror
! Local variable declarations
    integer(kind=SCOREP_InterimCommunicatorHandle) :: regionHandle
    logical :: event_gen_active
    logical :: event_gen_active_for_group
    integer(kind=c_int) :: scorep_in_measurement_save

    call scorep_in_measurement_increment()
    regionHandle = scorep_mpi_regions(SCOREP_MPI_REGION__MPI_ERROR_STRING)
    event_gen_active = logical(scorep_mpi_is_event_gen_on())
    event_gen_active_for_group = (logical(scorep_mpi_is_event_gen_on()) .and. (iand(scorep_mpi_enabled, SCOREP_MPI_ENABLED_ERR) .gt&
    &. 0))
    if (event_gen_active) then
        call scorep_mpi_event_gen_off()
        if (event_gen_active_for_group) then
            call SCOREP_EnterWrappedRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_EnterWrapper(regionHandle)
        end if
    end if

    call scorep_enter_wrapped_region(scorep_in_measurement_save)
    call PMPI_Error_string( &
        errorcode, &
        string, &
        resultlen, &
        internal_ierror)
    call scorep_exit_wrapped_region(scorep_in_measurement_save)

    if (event_gen_active) then
        if (event_gen_active_for_group) then
            call SCOREP_ExitRegion(regionHandle)
        else if (scorep_is_unwinding_enabled) then
            call SCOREP_ExitWrapper(regionHandle)
        end if
        call scorep_mpi_event_gen_on()
    end if

    if (present(ierror)) then
        ierror = internal_ierror
    end if
    call scorep_in_measurement_decrement()
end subroutine
#undef CHOICE_BUFFER_TYPE
#endif

!>
!>@}
!>
