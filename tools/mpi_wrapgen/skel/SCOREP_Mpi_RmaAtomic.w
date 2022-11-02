${guard:start}
/**
 * Measurement wrapper for ${name}
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_RmaAtomic.w
 * @note C interface
 * @note Introduced with MPI-${mpi:version}
 * @ingroup ${group|lowercase}
 * Triggers three events: Enter, RmaAtomic, and Leave.
 */
${proto:c}
{
  SCOREP_IN_MEASUREMENT_INCREMENT();
  const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
  const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR(SCOREP_MPI_ENABLED_${group|uppercase});
  scorep_mpi_rma_request* rma_request  = NULL;
  SCOREP_RmaWindowHandle  win_handle   = scorep_mpi_win_handle( win );
  SCOREP_MpiRequestId     matching_id  = 0;
  ${rtype} return_val;

  if (event_gen_active)
  {
    SCOREP_MPI_EVENT_GEN_OFF();
    if (event_gen_active_for_group)
    {
      SCOREP_EnterWrappedRegion(scorep_mpi_regions[SCOREP_MPI_REGION__${name|uppercase}]);
      if (target_rank != MPI_PROC_NULL)
      {
        uint64_t bytes_get = ${attribute(get_count)};
        uint64_t bytes_put = ${attribute(put_count)};

        /*
         * The following code block is only used if the prototype
         * definition specifies:
         *    <attribute id="rma_explicit_handle">true</attribute>
         */
#if ${attribute(rma_explicit_handle)}
        // is explicit request?
        // Explicit request always get a new matching id
        matching_id = scorep_mpi_get_request_id();
#else
        // Implicit request may already have a valid matching id
        rma_request = scorep_mpi_rma_request_find( win_handle,
                                                   target_rank,
                                                   MPI_REQUEST_NULL,
                                                   SCOREP_MPI_RMA_REQUEST_${attribute(rma_remote_completion)}_COMPLETION );

        if (rma_request == NULL)
        {
          matching_id = scorep_mpi_get_request_id();
        }
        else
        {
          matching_id = rma_request->matching_id;
        }
#endif

        SCOREP_RmaAtomic( win_handle, target_rank,
                          SCOREP_RMA_ATOMIC_TYPE_${attribute(atomic_op)|uppercase},
                          bytes_put, bytes_get, matching_id);
      }
    }
    else if ( SCOREP_IsUnwindingEnabled() )
    {
      SCOREP_EnterWrapper(scorep_mpi_regions[SCOREP_MPI_REGION__${name|uppercase}]);
    }
  }

  SCOREP_ENTER_WRAPPED_REGION();
  return_val = ${call:pmpi};
  SCOREP_EXIT_WRAPPED_REGION();

  if (event_gen_active)
  {
    if (event_gen_active_for_group)
    {
      if (target_rank != MPI_PROC_NULL)
      {
        /*
         * The following code block is only used if the prototype
         * definition specifies:
         *    <attribute id="rma_explicit_handle">true</attribute>
         */
#if ${attribute(rma_explicit_handle)}
        // rma_explicit_handle
        // Both implicit _and_ explicit request tracking needed.

        // Start implicit request tracking
        rma_request =
          scorep_mpi_rma_request_create( win_handle,
                                         target_rank,
                                         *request,
                                         SCOREP_MPI_RMA_REQUEST_${attribute(rma_remote_completion)}_COMPLETION,
                                         matching_id );

        // Start explicit request tracking
        scorep_mpi_request_win_create( *request, rma_request );
#else
        // Only implicit request tracking needed
        if (rma_request == NULL)
        {
          // Create a new request for key tuple
          // (win, target, completion)
          scorep_mpi_rma_request_create( win_handle,
                                         target_rank,
                                         MPI_REQUEST_NULL,
                                         SCOREP_MPI_RMA_REQUEST_${attribute(rma_remote_completion)}_COMPLETION,
                                         matching_id );
        }
#endif
      }
      SCOREP_ExitRegion(scorep_mpi_regions[SCOREP_MPI_REGION__${name|uppercase}]);
    }
    else if ( SCOREP_IsUnwindingEnabled() )
    {
      SCOREP_ExitWrapper(scorep_mpi_regions[SCOREP_MPI_REGION__${name|uppercase}]);
    }
    SCOREP_MPI_EVENT_GEN_ON();
  }

  SCOREP_IN_MEASUREMENT_DECREMENT();
  return return_val;
}
${guard:end}
