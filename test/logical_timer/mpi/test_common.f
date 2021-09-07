      subroutine keep_busy( n )
      implicit none
      integer, intent(in) :: n

      integer i
      do i = 1, n
          call do_something( )
      end do
      end subroutine keep_busy


      subroutine do_something ()
      implicit none
      end subroutine do_something


      subroutine late_send(comm, send_buf, recv_buf, src, dest, delay)
      implicit none
      include 'mpif.h'

      integer, intent(in) :: comm, send_buf, src, dest, delay
      integer, intent(out):: recv_buf

      integer:: rank
      integer:: tag, err

      call MPI_Comm_rank( comm, rank, err )
      tag = 0

      if ( rank .eq. src ) then
          call keep_busy( delay )
          call MPI_Send( send_buf, 1, MPI_INT, 1, tag, comm, err )
      elseif ( rank .eq. dest ) then
          call MPI_Recv( recv_buf, 1, MPI_INT,
     >                   0, tag, comm, MPI_STATUS_IGNORE, err )
      endif
      end subroutine late_send
