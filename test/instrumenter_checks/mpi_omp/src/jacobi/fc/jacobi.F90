#include <scorep/SCOREP_User.inc>
module JacobiMod
    use VariableDef
    implicit none

    contains

    subroutine Jacobi(myData)
        use mpi
        implicit none
        !********************************************************************
        ! Subroutine HelmholtzJ                                             *
        ! Solves poisson equation on rectangular grid assuming :            *
        ! (1) Uniform discretization in each direction, and                 *
        ! (2) Dirichlect boundary conditions                                *
        ! Jacobi method is used in this routine                             *
        !                                                                   *
        ! Input : n,m   Number of grid points in the X/Y directions         *
        !         dx,dy Grid spacing in the X/Y directions                  *
        !         alpha Helmholtz eqn. coefficient                          *
        !         omega Relaxation factor                                   *
        !         myData%afF(n,m) Right hand side function                  *
        !         myData%afU(n,m) Dependent variable/Solution               *
        !         tol    Tolerance for iterative solver                     *
        !         maxit  Maximum number of iterations                       *
        !                                                                   *
        ! Output : myData%afU(n,m) - Solution                               *
        !********************************************************************

        !.. Formal Arguments ..
        type(JacobiData), intent(inout) :: myData

        !.. Local Scalars ..
        integer :: i, j, iErr
        double precision :: ax, ay, b, residual(1), fLRes, tmpResd(1)

        !.. Local Arrays ..
        double precision, allocatable :: uold(:,:)

        !.. Pomp user region handle to test on the fly registration
        integer( kind=8 ) :: pomp_user_region_handle = 0;

        SCOREP_USER_REGION_DEFINE( main_loop )

        !.. Intrinsic Functions ..
        intrinsic DBLE, SQRT

        allocate(uold (0 : myData%iCols -1, myData%iRowFirst : myData%iRowLast))

        ! ... Executable Statements ...
        ! Initialize coefficients

        if (allocated(uold)) then
            ax = 1.0d0 / (myData%fDx * myData%fDx)      ! X-direction coef
            ay = 1.0d0 / (myData%fDx * myData%fDx)      ! Y-direction coef
            b = -2.0d0 * (ax + ay) - myData%fAlpha      ! Central coeff
            residual = 10.0d0 * myData%fTolerance

#ifdef SCOREP_POMP_USER
            ! POMP2 user instrumentation
            ! Not inserted as pragma to test on-the-fly registration.
            ! With Pragmas, the instrumenter would create initialization time
            ! initialization.
            call POMP2_Begin( pomp_user_region_handle, &
              "90*regionType=userRegion*sscl=jacobi.F90:63:63*escl=jacobi.F90:102:102*userRegionName=userloop**" )
#endif
            do while (myData%iIterCount < myData%iIterMax .and. residual(1) > myData%fTolerance)
                SCOREP_USER_REGION_BEGIN( main_loop, "main_loop", SCOREP_USER_REGION_TYPE_DYNAMIC )
                residual = 0.0d0

            ! Copy new solution into old
                call ExchangeJacobiMpiData(myData, uold)
!$omp parallel private(fLRes, tmpResd, i)
!$omp do reduction(+:residual)
                  ! Compute stencil, residual, & update
                   do j = myData%iRowFirst + 1, myData%iRowLast - 1
                       do i = 1, myData%iCols - 2
                           ! Evaluate residual
                           fLRes = (ax * (uold(i-1, j) + uold(i+1, j)) &
                                  + ay * (uold(i, j-1) + uold(i, j+1)) &
                                  + b * uold(i, j) - myData%afF(i, j)) / b

                           ! Update solution
                           myData%afU(i, j) = uold(i, j) - myData%fRelax * fLRes

                           ! Accumulate residual error
                           residual = residual + fLRes * fLRes
                       end do
                   end do
!$omp end do
!$omp end parallel
                  tmpResd = residual
                  call MPI_Allreduce(tmpResd, residual, 1, MPI_DOUBLE_PRECISION, &
                                MPI_SUM, MPI_COMM_WORLD, Ierr)

                 ! Error check
                 myData%iIterCount = myData%iIterCount + 1
                 residual = SQRT(residual) / DBLE(myData%iCols * myData%iRows)

                 SCOREP_USER_REGION_END( main_loop )
            ! End iteration loop
            end do

#ifdef SCOREP_POMP_USER
            call POMP2_End( pomp_user_region_handle );
#endif

            myData%fResidual = residual(1)
            deallocate(uold)
        else
           write (*,*) 'Error: cant allocate memory'
           call Finish(myData)
           stop
        end if
    end subroutine Jacobi

    subroutine ExchangeJacobiMpiData (myData, uold)
    !    use VariableDef
        use mpi
        implicit none
        type(JacobiData), intent(inout) :: myData
        double precision, intent(inout) :: uold(0: myData%iCols -1, myData%iRowFirst : myData%iRowLast)
        integer :: request(2), status(MPI_STATUS_SIZE, 2)
        integer :: single_status(MPI_STATUS_SIZE)
        integer, parameter :: iTagMoveLeft = 10, iTagMoveRight = 11
        integer i, j, iErr, iReqCnt
        double precision :: rbl(0:myData%iCols-1), rbr(0:myData%iCols-1), sbl(0:myData%iCols-1), sbr(0:myData%iCols-1)

        iReqCnt = 0
        if (myData%iMyRank /= myData%iNumProcs - 1) then
!!           /* send stripe mhi-1 to right neighbour async */
            iReqCnt = iReqCnt + 1
            sbr = myData%afU(:,myData%iRowLast-1)
            call MPI_Isend( sbr, myData%iCols, &
                            MPI_DOUBLE_PRECISION, myData%iMyRank + 1,         &
                            iTagMoveRight, MPI_COMM_WORLD, request(iReqCnt), iErr)
        end if

        if (myData%iMyRank /= 0) then
!!           /* send stripe mlo+1 to left neighbour async */
            iReqCnt = iReqCnt + 1
            sbl = myData%afU(:, myData%iRowFirst+1)
            call MPI_Isend( sbl, myData%iCols, &
                           MPI_DOUBLE_PRECISION, myData%iMyRank - 1,           &
                           iTagMoveLeft, MPI_COMM_WORLD, request(iReqCnt), iErr)
        end if

!$omp parallel
!$omp do
        do j = myData%iRowFirst + 1, myData%iRowLast - 1
            do i = 0, myData%iCols - 1
                uold(i, j) = myData%afU(i, j)
            end do
        end do
!$omp end do
!$omp end parallel

        if (myData%iMyRank /= 0) then
!!           /*  receive stripe mlo from left neighbour blocking */
            call MPI_Recv( rbl, myData%iCols, &
                           MPI_DOUBLE_PRECISION, myData%iMyRank - 1, &
                           iTagMoveRight, MPI_COMM_WORLD, single_status, iErr)
            uold(:, myData%iRowFirst) = rbl
        end if

        if (myData%iMyRank /= myData%iNumProcs - 1) then
!!           /* receive stripe mhi from right neighbour blocking */
            call MPI_Recv( rbr,  myData%iCols,  &
                           MPI_DOUBLE_PRECISION, myData%iMyRank + 1, &
                           iTagMoveLeft, MPI_COMM_WORLD, single_status, iErr)
            uold(:, myData%iRowLast) = rbr
        end if

        call MPI_Waitall(iReqCnt, request, status, iErr)
    end subroutine ExchangeJacobiMpiData
end module JacobiMod
