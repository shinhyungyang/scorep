      program send_recv
      implicit none
      include 'mpif.h'

      integer :: err

      integer :: rank_w, nprocs_w
      integer :: rank, nprocs
      integer :: tag = 0
      integer :: send_buf, recv_buf

      integer :: comm

      integer :: provided

c       call MPI_Init( err )
      call mpi_init_thread(MPI_THREAD_FUNNELED, provided, err)

      call MPI_Comm_size( MPI_COMM_WORLD, nprocs_w, err )
      call MPI_Comm_rank( MPI_COMM_WORLD, rank_w, err )

      call MPI_Comm_split ( MPI_COMM_WORLD,
     >                      rank_w/2, modulo( rank_w, 2 ), comm, err )
      call MPI_Comm_size( comm, nprocs, err )
      call MPI_Comm_rank( comm, rank, err )


      if ( nprocs .lt. 2 ) then
          write (*,*) "This test needs at least 2 processes to run"
          call MPI_Finalize( err )
          return
      endif

      send_buf = 0
      recv_buf = 0

      if ( rank .eq. 0 ) then
          send_buf = 999
      endif

c     send send_buf on 0 to recv_buf on 1, with a delay of 5
      call late_send( comm, send_buf, recv_buf, 0, 1, 5 )

      write (*,*) "Message buffers of rank ", rank_w, " of ", nprocs_w,
     >            " : Send = ", send_buf, " Recv = ", recv_buf

      call MPI_Finalize ( err )
      end program send_recv
