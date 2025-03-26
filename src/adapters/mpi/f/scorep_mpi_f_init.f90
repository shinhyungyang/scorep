!
!  This file is part of the Score-P software (http://www.score-p.org)
!
!  Copyright (c) 2009-2011,
!  RWTH Aachen University, Germany
!
!  Copyright (c) 2009-2011,
!  Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
!
!  Copyright (c) 2009-2011,
!  Technische Universitaet Dresden, Germany
!
!  Copyright (c) 2009-2011,
!  University of Oregon, Eugene, USA
!
!  Copyright (c) 2009-2011,
!  Forschungszentrum Juelich GmbH, Germany
!
!  Copyright (c) 2009-2011,
!  German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
!
!  Copyright (c) 2009-2011,
!  Technische Universitaet Muenchen, Germany
!
! This software may be modified and distributed under the terms of
! a BSD-style license. See the COPYING file in the package base
! directory for details.
!

!>
!! @file
!! @brief      Fortran functions, called during the initialization of the mpi adapter to
!!             obtain the value of MPI_STATUS_SIZE.
!!
!! @ingroup    MPI_Wrapper
!<

subroutine scorep_fortran_get_mpi_status_size(status_size)
    use mpi
    integer status_size
    status_size = MPI_STATUS_SIZE
end

subroutine scorep_fortran_get_mpi_bottom()
    use mpi
    call scorep_mpi_fortran_init_bottom(MPI_BOTTOM)
end

subroutine scorep_fortran_get_mpi_in_place()
    use mpi
    call scorep_mpi_fortran_init_in_place(MPI_IN_PLACE)
end

subroutine scorep_fortran_get_mpi_status_ignore()
    use mpi
    call scorep_mpi_fortran_init_status_ignore(MPI_STATUS_IGNORE)
end

subroutine scorep_fortran_get_mpi_statuses_ignore()
    use mpi
    call scorep_mpi_fortran_init_statuses_ignore(MPI_STATUSES_IGNORE)
end

subroutine scorep_fortran_get_mpi_unweighted()
    use mpi
    call scorep_mpi_fortran_init_unweighted(MPI_UNWEIGHTED)
end
