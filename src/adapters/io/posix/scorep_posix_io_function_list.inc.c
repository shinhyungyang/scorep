#ifndef SCOREP_POSIX_IO_PROCESS_FUNC
#error Macro 'SCOREP_POSIX_IO_PROCESS_FUNC' was not defined
#endif

/*
 * POSIX I/O routines
 */

#if HAVE( POSIX_IO_SYMBOL_CLOSE )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     close,        ( int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_CREAT )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     creat,        ( const char*, mode_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_CREAT64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     creat64,      ( const char*, mode_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_DUP )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     dup,          ( int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_DUP2 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     dup2,         ( int, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_DUP3 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     dup3,         ( int, int, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FCNTL )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     fcntl,        ( int, int, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FDATASYNC )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     fdatasync,    ( int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FSYNC )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     fsync,        ( int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_LOCKF )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     lockf,        ( int, int, off_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_LSEEK )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, off_t,   lseek,        ( int, off_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_LSEEK64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, scorep_off64_t, lseek64, ( int, scorep_off64_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_OPEN )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     open,         ( const char*, int, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_OPEN64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     open64,       ( const char*, int, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_OPENAT )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     openat,       ( int, const char*, int, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PREAD )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pread,        ( int, void*, size_t, off_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PREAD64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pread64,      ( int, void*, size_t, scorep_off64_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PREADV )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, preadv,       ( int, const struct iovec*, int, off_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PREADV2 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, preadv2,      ( int, const struct iovec*, int, off_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PREADV64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, preadv64,     ( int, const struct iovec*, int, scorep_off64_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PREADV64V2 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, preadv64v2,   ( int, const struct iovec*, int, scorep_off64_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PSELECT )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, WRAPPER,          int,     pselect,      ( int, fd_set*, fd_set*, fd_set*, const struct timespec*, const sigset_t* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PWRITE )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pwrite,       ( int, const void*, size_t, off_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PWRITE64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pwrite64,     ( int, const void*, size_t, scorep_off64_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PWRITEV )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pwritev,      ( int, const struct iovec*, int, off_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PWRITEV2 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pwritev2,     ( int, const struct iovec*, int, off_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PWRITEV64 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pwritev64,    ( int, const struct iovec*, int, scorep_off64_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PWRITEV64V2 )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, pwritev64v2,  ( int, const struct iovec*, int, scorep_off64_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_READ )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, read,         ( int, void*, size_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_READV )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, readv,        ( int, const struct iovec*, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_SELECT )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, WRAPPER,          int,     select,       ( int, fd_set*, fd_set*, fd_set*, struct timeval* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_SYNC )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          void,    sync,         ( ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_SYNCFS )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     syncfs,       ( int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_UNLINK )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     unlink,       ( const char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_UNLINKAT )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int,     unlinkat,     ( int, const char*, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_WRITE )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, write,        ( int, const void*, size_t ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_WRITEV )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          ssize_t, writev,       ( int, const struct iovec*, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_CLOSEDIR )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO_METADATA, int, closedir,         ( DIR* ) )
#endif
/*
 * POSIX Async I/O routines
 */

#if HAVE( POSIX_IO_SYMBOL_AIO_CANCEL )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     aio_cancel,   ( int, struct aiocb* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_AIO_ERROR )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     aio_error,    ( const struct aiocb* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_AIO_FSYNC )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     aio_fsync,    ( int, struct aiocb* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_AIO_READ )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     aio_read,     ( struct aiocb* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_AIO_RETURN )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, WRAPPER,          ssize_t, aio_return,   ( struct aiocb* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_AIO_SUSPEND )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, WRAPPER,          int,     aio_suspend,  ( const struct aiocb* const*, int, const struct timespec* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_AIO_WRITE )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     aio_write,    ( struct aiocb* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_LIO_LISTIO )
SCOREP_POSIX_IO_PROCESS_FUNC( POSIX_IO, FILE_IO,          int,     lio_listio,   ( int, struct aiocb* const*, int, struct sigevent* ) )
#endif

/*
 * ISO C I/O routines
 */

#if HAVE( POSIX_IO_SYMBOL_FCLOSE )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fclose,       ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FDOPEN )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   fdopen,       ( int, const char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FFLUSH )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fflush,       ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FGETC )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fgetc,        ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FGETPOS )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     WRAPPER,          int,     fgetpos,      ( FILE*, fpos_t* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FGETS )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          char*,   fgets,        ( char*, int, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FLOCKFILE )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          void,    flockfile,    ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FOPEN )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   fopen,        ( const char*, const char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FOPEN64 )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   fopen64,      ( const char*, const char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FPRINTF ) && HAVE( POSIX_IO_SYMBOL_VFPRINTF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fprintf,      ( FILE*, const char*, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FPUTC )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fputc,        ( int, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FPUTS )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fputs,        ( const char*, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FREAD )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          size_t,  fread,        ( void*, size_t, size_t, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FREOPEN )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, FILE*,   freopen,      ( const char*, const char*, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FSCANF ) && HAVE( POSIX_IO_SYMBOL_VFSCANF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     fscanf,       ( FILE*, const char*, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FSEEK )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fseek,        ( FILE*, long, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FSEEKO )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fseeko,       ( FILE*, off_t, int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FSEEKO64 )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fseeko64,     ( FILE*, scorep_off64_t, int ) )
#endif
// #if HAVE( POSIX_IO_SYMBOL_FSETPOS )
//SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fsetpos,      ( FILE*, const fpos_t* ) )
// #endif
// #if HAVE( POSIX_IO_SYMBOL_FSETPOS64 )
//SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     fsetpos64,    ( FILE*, const fpos64_t* ) )
// #endif
#if HAVE( POSIX_IO_SYMBOL_FTELL )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     WRAPPER,          long,    ftell,        ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FTELLO )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     WRAPPER,          off_t,   ftello,       ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FTRYLOCKFILE )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     ftrylockfile, ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FUNLOCKFILE )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          void,    funlockfile,  ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_FWRITE )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          size_t,  fwrite,       ( const void*, size_t, size_t, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_GETC )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     getc,         ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_GETCHAR )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     getchar,      ( ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_GETS )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          char*,   gets,         ( char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PRINTF ) && HAVE( POSIX_IO_SYMBOL_VPRINTF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     printf,       ( const char*, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PUTCHAR )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     putchar,      ( int ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_PUTS )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     puts,         ( const char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_REMOVE )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, int,     remove,       ( const char* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_REWIND )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO_METADATA, void,    rewind,       ( FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_SCANF ) && HAVE( POSIX_IO_SYMBOL_VSCANF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     scanf,        ( const char*, ... ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_UNGETC )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     ungetc,       ( int, FILE* ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_VFPRINTF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vfprintf,     ( FILE*, const char*, va_list ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_VFSCANF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vfscanf,      ( FILE*, const char*, va_list ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_VPRINTF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vprintf,      ( const char*, va_list ) )
#endif
#if HAVE( POSIX_IO_SYMBOL_VSCANF )
SCOREP_POSIX_IO_PROCESS_FUNC( ISOC,     FILE_IO,          int,     vscanf,       ( const char*, va_list ) )
#endif

#undef SCOREP_POSIX_IO_PROCESS_FUNC
