##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2013,
## RWTH Aachen University, Germany
##
## Copyright (c) 2009-2013,
## Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##
## Copyright (c) 2009-2014, 2019,
## Technische Universitaet Dresden, Germany
##
## Copyright (c) 2009-2013,
## University of Oregon, Eugene, USA
##
## Copyright (c) 2009-2015, 2023, 2025,
## Forschungszentrum Juelich GmbH, Germany
##
## Copyright (c) 2009-2013,
## German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##
## Copyright (c) 2009-2013,
## Technische Universitaet Muenchen, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license. See the COPYING file in the package base
## directory for details.
##
AC_DEFUN([_SCOREP_MPI_COMPLIANCE_CHECKS], [
    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Address], [void*, location, MPI_Aint* address],
    [CONST],       [const void*, location, MPI_Aint*, address])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Type_hindexed], [int, count, int*, array_of_blocklengths, MPI_Aint*, array_of_displacements, MPI_Datatype, oldtype, MPI_Datatype*, newtype],
    [CONST],             [int, count, const int*, array_of_blocklengths, const MPI_Aint*, array_of_displacements, MPI_Datatype, oldtype, MPI_Datatype*, newtype])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Type_struct], [int, count, int*, array_of_blocklengths, MPI_Aint*, array_of_displacements, MPI_Datatype*, array_of_types, MPI_Datatype*, newtype],
    [CONST],           [int, count, const int*, array_of_blocklengths, const MPI_Aint*, array_of_displacements, const MPI_Datatype*, array_of_types, MPI_Datatype*, newtype])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Add_error_string], [int, errorcode, char*, string],
    [CONST],                [int, errorcode, const char*, string])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Compare_and_swap], [void*, origin_addr, void*, compare_addr, void*, result_addr, MPI_Datatype, datatype, int, target_rank, MPI_Aint, target_disp, MPI_Win, win],
    [CONST],                [const void*, origin_addr, const void*, compare_addr, void*, result_addr, MPI_Datatype, datatype, int, target_rank, MPI_Aint, target_disp, MPI_Win, win])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Fetch_and_op], [void*, origin_addr, void*, result_addr, MPI_Datatype, datatype, int, target_rank, MPI_Aint, target_disp, MPI_Op, op, MPI_Win, win],
    [CONST],            [const void*, origin_addr, void*, result_addr, MPI_Datatype, datatype, int, target_rank, MPI_Aint, target_disp, MPI_Op, op, MPI_Win, win])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Raccumulate], [void*, origin_addr, int, origin_count, MPI_Datatype, origin_datatype, int, target_rank, MPI_Aint, target_disp, int, target_count, MPI_Datatype, target_datatype, MPI_Op, op, MPI_Win, win, MPI_Request*, request],
    [CONST],           [const void*, origin_addr, int, origin_count, MPI_Datatype, origin_datatype, int, target_rank, MPI_Aint, target_disp, int, target_count, MPI_Datatype, target_datatype, MPI_Op, op, MPI_Win, win, MPI_Request*, request])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Win_detach], [MPI_Win, win, void*, base],
    [CONST],          [MPI_Win, win, const void*, base])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Psend_init], [const void*, buf, int, partitions, MPI_Count, count, MPI_Datatype, datatype, int, dest, int, tag, MPI_Comm, comm, MPI_Info, info, MPI_Request*, request],
    [NOT_CONST],      [void*, buf, int, partitions, MPI_Count, count, MPI_Datatype, datatype, int, dest, int, tag, MPI_Comm, comm, MPI_Info, info, MPI_Request*, request])

    _SCOREP_MPI_CHECK_COMPLIANCE(
    [int], [return 0],
    [MPI_Pready_list], [int, length, const int*, array_of_partitions, MPI_Request, request],
    [NOT_CONST],       [int, length, int*, array_of_partitions, MPI_Request, request])
])
