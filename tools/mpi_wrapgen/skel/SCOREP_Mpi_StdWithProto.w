${guard:start}
/**
 * Measurement wrapper for ${name}
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdWithProto.w
 * @note C interface
 * @note Introduced with MPI-${mpi:version}
 * @ingroup ${group|lowercase}
 * Triggers an enter and exit event.
 * It wraps the ${name} call with enter and exit events.
 */
${rtype}
${name} SCOREP_${name|uppercase}_PROTO_ARGS
{
    ${rtype} return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_${group|uppercase} ) )
    {
        ${decl}
        ${init}

        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__${name|uppercase} ] );

        ${xblock}
        return_val = ${call:pmpi};

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__${name|uppercase} ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = ${call:pmpi};
    }

    return return_val;
}
${guard:end}

