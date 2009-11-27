${guard:start}
/**
 * Measurement wrapper for ${name}
 * @note Auto-generated by wrapgen from template: SILC_Mpi_PtpSendinit.w
 * @note C interface
 * @note Introduced with MPI-${mpi:version}
 * @ingroup ${group|lowercase}
 * Sequence of events:
 * @li enter region '$(name)'
 * @li exit region '$(name)'
 */
${proto:c}
{
  const int event_gen_active = SILC_MPI_IS_EVENT_GEN_ON_FOR(SILC_MPI_ENABLED_${group|uppercase});
  int return_val, sz;

  if (event_gen_active)
    {
      SILC_MPI_EVENT_GEN_OFF();
      SILC_EnterRegion(silc_mpi_regid[SILC__${name|uppercase}]);
    }

  PMPI_Type_size(datatype, &sz);

  return_val = ${call:pmpi};
/*
  if (dest != MPI_PROC_NULL && return_val == MPI_SUCCESS)
    silc_request_create(*request, (ERF_SEND | ERF_IS_PERSISTENT),
                       tag, dest, count*sz, datatype, comm);
*/
  if (event_gen_active)
    {
      SILC_ExitRegion(silc_mpi_regid[SILC__${name|uppercase}]);
      SILC_MPI_EVENT_GEN_ON();
    }

  return return_val;
}
${guard:end}
