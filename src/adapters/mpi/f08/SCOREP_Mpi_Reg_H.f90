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

!>
!> @file
!> @ingroup    MPI_Wrapper
!>
!> @brief This file is currently a placeholder
!>

module SCOREP_Mpi_Reg_H
    use :: SCOREP_PublicTypes_H, only:SCOREP_RegionHandle

    implicit none

    private :: SCOREP_RegionHandle

    integer, parameter :: SCOREP_MPI_NUM_REGIONS = 427

    integer(SCOREP_RegionHandle), bind(c, name="scorep_mpi_regions") :: scorep_mpi_regions(SCOREP_MPI_NUM_REGIONS)
end module
