program main
    use :: mpi_f08
    implicit none

    integer :: ierr
    type(MPI_Info) :: info
    character(len=:), allocatable :: key1, key2, buf
    integer :: buflen
    logical :: flag

    call MPI_Init(ierr)

    call MPI_Info_create(info)

    key1 = "key1"
    key2 = "key2"

    call MPI_Info_set(info, key1, "value_1")
    call MPI_Info_set(info, key2, "value_2")

    call MPI_Info_set(info, key1, "Congratulations: You finished the tutorial!")

    call MPI_Info_get_valuelen(info, key1, buflen, flag)
    if (flag) then
        allocate (character(len=buflen) :: buf)
        call MPI_Info_get(info, key1, buflen, buf, flag)
        print '("Info[", a, "] = ", a)', key1, buf
    else
        print '("Info[", a, "] not set")', key1
    end if

    call MPI_Finalize(ierr)

end program main
