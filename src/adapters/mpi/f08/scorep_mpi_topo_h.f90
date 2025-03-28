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
module scorep_mpi_topo_h
    use, intrinsic :: iso_c_binding, only: c_char, C_NULL_CHAR
    use :: mpi_f08, only:MPI_Comm

    use :: scorep_mpi_f08_utils, only:c_string

    implicit none

    private

    public :: scorep_mpi_topo_create_cart_definition

    interface scorep_mpi_topo_create_cart_definition
        module procedure scorep_mpi_topo_create_cart_definition
    end interface

    interface
        subroutine scorep_mpi_topo_create_cart_definition_toC(namePrefix, cartComm) &
            bind(c, name="scorep_mpi_topo_create_cart_definition_fromF08")
            import
            implicit none
            character(kind=c_char), intent(in) :: namePrefix(*)
            integer, intent(in) :: cartComm
        end subroutine
    end interface

contains

    subroutine scorep_mpi_topo_create_cart_definition(namePrefix, cartComm)
        character(len=*), intent(in) :: namePrefix
        type(MPI_Comm), intent(in) :: cartComm

        character(kind=c_char, len=len(namePrefix) + 1) :: name_prefix_c

        call c_string(namePrefix, name_prefix_c)
        call scorep_mpi_topo_create_cart_definition_toC(name_prefix_c, cartComm%MPI_VAL)
    end subroutine

end module
