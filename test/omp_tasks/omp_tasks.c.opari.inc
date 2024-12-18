#include <opari2/pomp2_lib.h>
#define POMP2_DLIST_00001 shared( opari2_region_1 )
static OPARI2_Region_handle opari2_region_1 = ( OPARI2_Region_handle )0;
#define opari2_ctc_1 "168*regionType=task*sscl=omp_tasks.c:44:44*escl=omp_tasks.c:47:47**"
static OPARI2_Region_handle opari2_region_2 = ( OPARI2_Region_handle )0;
#define opari2_ctc_2 "172*regionType=taskwait*sscl=omp_tasks.c:49:49*escl=omp_tasks.c:49:49**"
#define POMP2_DLIST_00003 shared( opari2_region_3, opari2_region_4 )
static OPARI2_Region_handle opari2_region_3 = ( OPARI2_Region_handle )0;
#define opari2_ctc_3 "172*regionType=parallel*sscl=omp_tasks.c:58:58*escl=omp_tasks.c:62:62**"
static OPARI2_Region_handle opari2_region_4 = ( OPARI2_Region_handle )0;
#define opari2_ctc_4 "170*regionType=single*sscl=omp_tasks.c:60:60*escl=omp_tasks.c:61:61**"

#ifdef __cplusplus
extern "C"
#endif
void
POMP2_Init_reg_lihdix7loxm2_4()
{
    POMP2_Assign_handle( &opari2_region_1, opari2_ctc_1 );
    POMP2_Assign_handle( &opari2_region_2, opari2_ctc_2 );
    POMP2_Assign_handle( &opari2_region_3, opari2_ctc_3 );
    POMP2_Assign_handle( &opari2_region_4, opari2_ctc_4 );
}
