#ifndef SCOREP_POSIX_IO_PROCESS_FUNC
#error Macro 'SCOREP_POSIX_IO_PROCESS_FUNC' was not defined
#endif

/*
 * POSIX I/O routines
 */

SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     close,        ( int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     creat,        ( const char*, mode_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     creat64,      ( const char*, mode_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     dup,          ( int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     dup2,         ( int, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     dup3,         ( int, int, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     fcntl,        ( int, int, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     fdatasync,    ( int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     fsync,        ( int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     lockf,        ( int, int, off_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, off_t,   lseek,        ( int, off_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, scorep_off64_t, lseek64, ( int, scorep_off64_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     open,         ( const char*, int, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     open64,       ( const char*, int, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     openat,       ( int, const char*, int, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pread,        ( int, void*, size_t, off_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pread64,      ( int, void*, size_t, scorep_off64_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, preadv,       ( int, const struct iovec*, int, off_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, preadv2,      ( int, const struct iovec*, int, off_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, preadv64,     ( int, const struct iovec*, int, scorep_off64_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, preadv64v2,   ( int, const struct iovec*, int, scorep_off64_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, WRAPPER,          int,     pselect,      ( int, fd_set*, fd_set*, fd_set*, const struct timespec*, const sigset_t* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pwrite,       ( int, const void*, size_t, off_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pwrite64,     ( int, const void*, size_t, scorep_off64_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pwritev,      ( int, const struct iovec*, int, off_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pwritev2,     ( int, const struct iovec*, int, off_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pwritev64,    ( int, const struct iovec*, int, scorep_off64_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, pwritev64v2,  ( int, const struct iovec*, int, scorep_off64_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, read,         ( int, void*, size_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, readv,        ( int, const struct iovec*, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, WRAPPER,          int,     select,       ( int, fd_set*, fd_set*, fd_set*, struct timeval* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          void,    sync,         ( ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     syncfs,       ( int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     unlink,       ( const char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int,     unlinkat,     ( int, const char*, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, write,        ( int, const void*, size_t ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          ssize_t, writev,       ( int, const struct iovec*, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO_METADATA, int, closedir,         ( DIR* ) )
/*
 * POSIX Async I/O routines
 */

SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     aio_cancel,   ( int, struct aiocb* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     aio_error,    ( const struct aiocb* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     aio_fsync,    ( int, struct aiocb* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     aio_read,     ( struct aiocb* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, WRAPPER,          ssize_t, aio_return,   ( struct aiocb* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, WRAPPER,          int,     aio_suspend,  ( const struct aiocb* const*, int, const struct timespec* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     aio_write,    ( struct aiocb* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX, FILE_IO,          int,     lio_listio,   ( int, struct aiocb* const*, int, struct sigevent* ) )

/*
 * ISO C I/O routines
 */

SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fclose,       ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   fdopen,       ( int, const char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fflush,       ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fgetc,        ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     WRAPPER,          int,     fgetpos,      ( FILE*, fpos_t* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          char*,   fgets,        ( char*, int, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          void,    flockfile,    ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   fopen,        ( const char*, const char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   fopen64,      ( const char*, const char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fprintf,      ( FILE*, const char*, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fputc,        ( int, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fputs,        ( const char*, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          size_t,  fread,        ( void*, size_t, size_t, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   freopen,      ( const char*, const char*, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fscanf,       ( FILE*, const char*, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fseek,        ( FILE*, long, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fseeko,       ( FILE*, off_t, int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fseeko64,     ( FILE*, scorep_off64_t, int ) )
//SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fsetpos,      ( FILE*, const fpos_t* ) )
//SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fsetpos64,    ( FILE*, const fpos64_t* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     WRAPPER,          long,    ftell,        ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     WRAPPER,          off_t,   ftello,       ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     ftrylockfile, ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          void,    funlockfile,  ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          size_t,  fwrite,       ( const void*, size_t, size_t, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     getc,         ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     getchar,      ( ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          char*,   gets,         ( char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     printf,       ( const char*, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     putchar,      ( int ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     puts,         ( const char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     remove,       ( const char* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, void,    rewind,       ( FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     scanf,        ( const char*, ... ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     ungetc,       ( int, FILE* ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vfprintf,     ( FILE*, const char*, va_list ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vfscanf,      ( FILE*, const char*, va_list ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vprintf,      ( const char*, va_list ) )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vscanf,       ( const char*, va_list ) )

#undef SCOREP_POSIX_IO_PROCESS_FUNC
