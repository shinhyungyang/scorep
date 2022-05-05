#define shpalloc_L shpalloc
#define shpalloc_U SHPALLOC

#define shpdeallc_L shpdeallc
#define shpdeallc_U SHPDEALLC

#define shpclmove_L shpclmove
#define shpclmove_U SHPCLMOVE


void
FSUB( shpalloc )( void** addr,
                  int*   length,
                  long*  errcode,
                  int*   abort )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shpalloc );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    pshpalloc( addr, length, errcode, abort );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_shmem_region__shpalloc );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}


void
FSUB( shpdealloc )( void** addr,
                    long*  errcode,
                    int*   abort )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shpdealloc );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    pshpdealloc( addr, errcode, abort );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_shmem_region__shpdealloc );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}


void
FSUB( shpclmove )( void* addr,
                   int*  length,
                   long* errcode,
                   int*  abort )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shpclmove );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    pshpclmove( addr, length, errcode, abort );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_shmem_region__shpclmove );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}
