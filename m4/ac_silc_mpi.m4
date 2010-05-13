## -*- mode: autoconf -*-

## 
## This file is part of the SILC project (http://www.silc.de)
##
## Copyright (c) 2009-2011, 
##    RWTH Aachen, Germany
##    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##    Technische Universitaet Dresden, Germany
##    University of Oregon, Eugene USA
##    Forschungszentrum Juelich GmbH, Germany
##    Technische Universitaet Muenchen, Germany
##
## See the COPYING file in the package base directory for details.
##

## file       ac_silc_mpi.m4
## maintainer Christian Roessel <c.roessel@fz-juelich.de>

AC_DEFUN([AC_SILC_MPI], [

AC_LANG_PUSH([C])
AX_MPI([silc_mpi_c_supported="yes"],[silc_mpi_c_supported="no"])
AC_LANG_POP([C])

# C++ MPI is needed for the jacobi C++ tests only.
AC_LANG_PUSH([C++])
AX_MPI([silc_mpi_cxx_supported="yes"],[silc_mpi_cxx_supported="no"])
AC_LANG_POP([C++])

AC_LANG_PUSH([Fortran 77])
AX_MPI([silc_mpi_f77_supported="yes"],[silc_mpi_f77_supported="no"])
AC_LANG_POP([Fortran 77])

AC_LANG_PUSH([Fortran])
AX_MPI([silc_mpi_f90_supported="yes"],[silc_mpi_f90_supported="no"])
AC_LANG_POP([Fortran])

if test "x${silc_mpi_c_supported}" = "xyes"; then
  if test "x${silc_mpi_f77_supported}" = "xyes" -o "x${silc_mpi_f90_supported}" = "xyes"; then
    silc_mpi_supported="yes"
  else
    silc_mpi_supported="no"
  fi 
else
   silc_mpi_supported="no"
fi

if test "x${silc_mpi_supported}" = "xno"; then
  AC_MSG_WARN([Non suitbale MPI compilers found. SILC MPI and hybrid libraries will not be build.])
fi
AM_CONDITIONAL([MPI_SUPPORTED], [test "x${silc_mpi_supported}" = "xyes"])


if test "x${silc_mpi_supported}" = "xyes"; then

  ac_silc_mpi_save_CC="$CC"
  CC="$MPICC"


  AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM(
          [[#include <mpi.h>]],
          [[#if (MPI_VERSION == 1) && (MPI_SUBVERSION == 2)
                double version = 1.2;
            #else
                not version 1.2
            #endif
          ]]
      )],
      [AC_DEFINE([HAVE_MPI_VERSION], [1], [[]])
       AC_DEFINE([HAVE_MPI_SUBVERSION], [2], [[]])
       AC_SUBST([HAVE_MPI_VERSION], [1])
       AC_SUBST([HAVE_MPI_SUBVERSION], [2])], 
      [
      AC_COMPILE_IFELSE(
          [AC_LANG_PROGRAM(
              [[#include <mpi.h>]],
              [[#if (MPI_VERSION == 2) && (MPI_SUBVERSION == 0)
                    double version = 2.0;
                #else
                    not version 2.0
                #endif
              ]]
          )],
          [AC_DEFINE([HAVE_MPI_VERSION], [2], [[]])
           AC_DEFINE([HAVE_MPI_SUBVERSION], [0], [[]])
           AC_SUBST([HAVE_MPI_VERSION], [2])
           AC_SUBST([HAVE_MPI_SUBVERSION], [0])], 
          [
          AC_COMPILE_IFELSE(
              [AC_LANG_PROGRAM(
                  [[#include <mpi.h>]],
                  [[#if (MPI_VERSION == 2) && (MPI_SUBVERSION == 1)
                        double version = 2.1;
                    #else
                        not version 2.1
                    #endif
                  ]]
              )],
              [AC_DEFINE([HAVE_MPI_VERSION], [2], [[]])
               AC_DEFINE([HAVE_MPI_SUBVERSION], [1], [[]])
               AC_SUBST([HAVE_MPI_VERSION], [2])
               AC_SUBST([HAVE_MPI_SUBVERSION], [1])], 
              [
              AC_COMPILE_IFELSE(
                  [AC_LANG_PROGRAM(
                      [[#include <mpi.h>]],
                      [[#if (MPI_VERSION == 2) && (MPI_SUBVERSION == 2)
                            double version = 2.2;
                        #else
                            not version 2.2
                        #endif
                      ]]
                   )],
                  [AC_DEFINE([HAVE_MPI_VERSION], [2], [[]])
                   AC_DEFINE([HAVE_MPI_SUBVERSION], [2], [[]])
                   AC_SUBST([HAVE_MPI_VERSION], [2])
                   AC_SUBST([HAVE_MPI_SUBVERSION], [2])], 
                  [

                  ]
              ) 
              ]
          )   
          ]
      )
      ]
  )


  AC_CHECK_DECLS([PMPI_Abort, PMPI_Accumulate, PMPI_Add_error_class, PMPI_Add_error_code, PMPI_Add_error_string, PMPI_Address, PMPI_Allgather, PMPI_Allgatherv, PMPI_Alloc_mem, PMPI_Allreduce, PMPI_Alltoall, PMPI_Alltoallv, PMPI_Alltoallw, PMPI_Attr_delete, PMPI_Attr_get, PMPI_Attr_put, PMPI_Barrier, PMPI_Bcast, PMPI_Bsend, PMPI_Bsend_init, PMPI_Buffer_attach, PMPI_Buffer_detach, PMPI_Cancel, PMPI_Cart_coords, PMPI_Cart_create, PMPI_Cart_get, PMPI_Cart_map, PMPI_Cart_rank, PMPI_Cart_shift, PMPI_Cart_sub, PMPI_Cartdim_get, PMPI_Close_port, PMPI_Comm_accept, PMPI_Comm_c2f, PMPI_Comm_call_errhandler, PMPI_Comm_compare, PMPI_Comm_connect, PMPI_Comm_create, PMPI_Comm_create_errhandler, PMPI_Comm_create_keyval, PMPI_Comm_delete_attr, PMPI_Comm_disconnect, PMPI_Comm_dup, PMPI_Comm_f2c, PMPI_Comm_free, PMPI_Comm_free_keyval, PMPI_Comm_get_attr, PMPI_Comm_get_errhandler, PMPI_Comm_get_name, PMPI_Comm_get_parent, PMPI_Comm_group, PMPI_Comm_join, PMPI_Comm_rank, PMPI_Comm_remote_group, PMPI_Comm_remote_size, PMPI_Comm_set_attr, PMPI_Comm_set_errhandler, PMPI_Comm_set_name, PMPI_Comm_size, PMPI_Comm_spawn, PMPI_Comm_spawn_multiple, PMPI_Comm_split, PMPI_Comm_test_inter, PMPI_Dims_create, PMPI_Dist_graph_create, PMPI_Dist_graph_create_adjacent, PMPI_Dist_graph_neighbors, PMPI_Dist_graph_neighbors_count, PMPI_Errhandler_create, PMPI_Errhandler_free, PMPI_Errhandler_get, PMPI_Errhandler_set, PMPI_Error_class, PMPI_Error_string, PMPI_Exscan, PMPI_File_c2f, PMPI_File_call_errhandler, PMPI_File_close, PMPI_File_create_errhandler, PMPI_File_delete, PMPI_File_f2c, PMPI_File_get_amode, PMPI_File_get_atomicity, PMPI_File_get_byte_offset, PMPI_File_get_errhandler, PMPI_File_get_group, PMPI_File_get_info, PMPI_File_get_position, PMPI_File_get_position_shared, PMPI_File_get_size, PMPI_File_get_type_extent, PMPI_File_get_view, PMPI_File_iread, PMPI_File_iread_at, PMPI_File_iread_shared, PMPI_File_iwrite, PMPI_File_iwrite_at, PMPI_File_iwrite_shared, PMPI_File_open, PMPI_File_preallocate, PMPI_File_read, PMPI_File_read_all, PMPI_File_read_all_begin, PMPI_File_read_all_end, PMPI_File_read_at, PMPI_File_read_at_all, PMPI_File_read_at_all_begin, PMPI_File_read_at_all_end, PMPI_File_read_ordered, PMPI_File_read_ordered_begin, PMPI_File_read_ordered_end, PMPI_File_read_shared, PMPI_File_seek, PMPI_File_seek_shared, PMPI_File_set_atomicity, PMPI_File_set_errhandler, PMPI_File_set_info, PMPI_File_set_size, PMPI_File_set_view, PMPI_File_sync, PMPI_File_write, PMPI_File_write_all, PMPI_File_write_all_begin, PMPI_File_write_all_end, PMPI_File_write_at, PMPI_File_write_at_all, PMPI_File_write_at_all_begin, PMPI_File_write_at_all_end, PMPI_File_write_ordered, PMPI_File_write_ordered_begin, PMPI_File_write_ordered_end, PMPI_File_write_shared, PMPI_Finalize, PMPI_Finalized, PMPI_Free_mem, PMPI_Gather, PMPI_Gatherv, PMPI_Get, PMPI_Get_address, PMPI_Get_count, PMPI_Get_elements, PMPI_Get_processor_name, PMPI_Get_version, PMPI_Graph_create, PMPI_Graph_get, PMPI_Graph_map, PMPI_Graph_neighbors, PMPI_Graph_neighbors_count, PMPI_Graphdims_get, PMPI_Grequest_complete, PMPI_Grequest_start, PMPI_Group_c2f, PMPI_Group_compare, PMPI_Group_difference, PMPI_Group_excl, PMPI_Group_f2c, PMPI_Group_free, PMPI_Group_incl, PMPI_Group_intersection, PMPI_Group_range_excl, PMPI_Group_range_incl, PMPI_Group_rank, PMPI_Group_size, PMPI_Group_translate_ranks, PMPI_Group_union, PMPI_Ibsend, PMPI_Info_c2f, PMPI_Info_create, PMPI_Info_delete, PMPI_Info_dup, PMPI_Info_f2c, PMPI_Info_free, PMPI_Info_get, PMPI_Info_get_nkeys, PMPI_Info_get_nthkey, PMPI_Info_get_valuelen, PMPI_Info_set, PMPI_Init, PMPI_Init_thread, PMPI_Initialized, PMPI_Intercomm_create, PMPI_Intercomm_merge, PMPI_Iprobe, PMPI_Irecv, PMPI_Irsend, PMPI_Is_thread_main, PMPI_Isend, PMPI_Issend, PMPI_Keyval_create, PMPI_Keyval_free, PMPI_Lookup_name, PMPI_Op_c2f, PMPI_Op_commutative, PMPI_Op_create, PMPI_Op_f2c, PMPI_Op_free, PMPI_Open_port, PMPI_Pack, PMPI_Pack_external, PMPI_Pack_external_size, PMPI_Pack_size, PMPI_Probe, PMPI_Publish_name, PMPI_Put, PMPI_Query_thread, PMPI_Recv, PMPI_Recv_init, PMPI_Reduce, PMPI_Reduce_local, PMPI_Reduce_scatter, PMPI_Reduce_scatter_block, PMPI_Register_datarep, PMPI_Request_c2f, PMPI_Request_f2c, PMPI_Request_free, PMPI_Request_get_status, PMPI_Rsend, PMPI_Rsend_init, PMPI_Scan, PMPI_Scatter, PMPI_Scatterv, PMPI_Send, PMPI_Send_init, PMPI_Sendrecv, PMPI_Sendrecv_replace, PMPI_Sizeof, PMPI_Ssend, PMPI_Ssend_init, PMPI_Start, PMPI_Startall, PMPI_Status_c2f, PMPI_Status_f2c, PMPI_Status_set_cancelled, PMPI_Status_set_elements, PMPI_Test, PMPI_Test_cancelled, PMPI_Testall, PMPI_Testany, PMPI_Testsome, PMPI_Topo_test, PMPI_Type_c2f, PMPI_Type_commit, PMPI_Type_contiguous, PMPI_Type_create_darray, PMPI_Type_create_f90_complex, PMPI_Type_create_f90_integer, PMPI_Type_create_f90_real, PMPI_Type_create_hindexed, PMPI_Type_create_hvector, PMPI_Type_create_indexed_block, PMPI_Type_create_keyval, PMPI_Type_create_resized, PMPI_Type_create_struct, PMPI_Type_create_subarray, PMPI_Type_delete_attr, PMPI_Type_dup, PMPI_Type_extent, PMPI_Type_f2c, PMPI_Type_free, PMPI_Type_free_keyval, PMPI_Type_get_attr, PMPI_Type_get_contents, PMPI_Type_get_envelope, PMPI_Type_get_extent, PMPI_Type_get_name, PMPI_Type_get_true_extent, PMPI_Type_hindexed, PMPI_Type_hvector, PMPI_Type_indexed, PMPI_Type_lb, PMPI_Type_match_size, PMPI_Type_set_attr, PMPI_Type_set_name, PMPI_Type_size, PMPI_Type_struct, PMPI_Type_ub, PMPI_Type_vector, PMPI_Unpack, PMPI_Unpack_external, PMPI_Unpublish_name, PMPI_Wait, PMPI_Waitall, PMPI_Waitany, PMPI_Waitsome, PMPI_Win_c2f, PMPI_Win_call_errhandler, PMPI_Win_complete, PMPI_Win_create, PMPI_Win_create_errhandler, PMPI_Win_create_keyval, PMPI_Win_delete_attr, PMPI_Win_f2c, PMPI_Win_fence, PMPI_Win_free, PMPI_Win_free_keyval, PMPI_Win_get_attr, PMPI_Win_get_errhandler, PMPI_Win_get_group, PMPI_Win_get_name, PMPI_Win_lock, PMPI_Win_post, PMPI_Win_set_attr, PMPI_Win_set_errhandler, PMPI_Win_set_name, PMPI_Win_start, PMPI_Win_test, PMPI_Win_unlock, PMPI_Win_wait, PMPI_Wtick, PMPI_Wtime], [], [], [[#include <mpi.h>]])

  CC="$ac_silc_mpi_save_CC"

fi # if test "x${silc_mpi_supported}" = "xyes"
])
