Usage
=====

    scorep-libwrap-init --name <wrappername> -x <language> [options] \
        [workspacedir]

Initializes the working directory for creating a user library wrapper,
named `wrappername`, into `workspacedir`. The final wrapper will be installed
into `directory`. `language` has to be either c or c++ plus optionally a
standard, e.g., c++11. The default workspace is the current directory.

Options
-------

  - `--prefix <directory>` Directory where to install the wrapper into.
                           Defaults to Score-P's installation directory.
  - `--display-name`       A more descriptive name for this wrapper. Can
                           contain spaces and special characters. Defaults to
                           `--name`.
  - `--backend <which>`    Backend compiler to be used for building the wrapper.
                           Has to be either vanilla, mpi or shmem.
  - `--cppflags "<flags>"` Compiler flags for the to-be-wrapped library
  - `--ldflags "<flags>"`  Linker flags for the to-be-wrapped library
  - `--libs "<libraries>"` To-be-wrapped libraries and their dependencies,
                           e.g., "-lm -lgmp".
  - `--update`             Do not exit if the working directory is already
                           initialized. Overwrites `Makefile` and creates any
                           file that is not already present.

  - `-h, --help`           Display this help message

Overview
========

User library wrapping enables users to install library wrappers for any
C/C++ library your application is using without recompiling the library or
application itself.

In contrast to wrappers integrated into Score-P, e.g., Pthreads, MPI, and
OpenCL, user library wrappers do not extract semantic information from the
library. It only intercepts function calls and provides their timing
information.

Without this mechanism, in order to intercept calls to a library, users
need to either build this library with Score-P or add manual instrumentation
to the application using the target library.
Another advantage of user library wrapping is you don't need access to the
source code of the target library. Headers and library files suffice.

Requirements:
--------------

To enable this feature, Score-P needs to be built with `libclang` from the LLVM
compiler infrastructure.
More details can be found in Score-P's `INSTALL` file under "User Library Wrapping".

You can find out whether user library wrapping is enabled in the configure
summary or via `scorep-info config-summary` in Section "Score-P (libwrap)".

Workflow for wrapping a library
===============================

Step 1: Initialize the working directory
----------------------------------------

To initialize the working directory for a new library wrapper use the
`scorep-libwrap-init` command. `scorep-libwrap-init` has a number of
obligatory and optional arguments. You need to supply:

  - A name for the wrapper library via `--name`, and
  - The used programming language (either `c` or `c++`) via `-x`.
    Optionally you can add a specific standard, e.g., `c++11`.

Optional arguments:

  - The last argument for `scorep-libwrap-init` specifies the working directory.
    It defaults to the current directory.
  - `--prefix` contains the installation prefix. It defaults to Score-P's
    installation directory.
  - `--display-name` contains a more descriptive name for the wrapper.
  - `--backend` lets you choose the backend compiler between vanilla/none,
    mpi and shmem. If applications using the library use a special compiler
    like `mpicc` or `oshcc`, you have to change that value to the corresponding
    backend.
  - `--cppflags` contains the preprocessing/compiler flags required to compile
    an application using the target library.
  - `--ldflags` contains the linker flags required to link an application using
    the target library.
  - `--libs` contains the target library and libraries it depends on as a
    space-separated list of `-l`-flags.
  - `--update` overwrites `Makefile` in order to update the current wrapper,
    instead of exiting due to the working directory already being initialized.

Example:

    $ scorep-libwrap-init --name=mylib --display-name="My Lib" -x c \
        --cppflags="-DSOME_DEFINE -I/opt/mylib/include -I/opt/somelib/include" \
        --ldflags="-L/opt/mylib/lib -L/opt/somelib/lib" \
        --libs="-lmylib -lsomelib -lz -lm" \
        $(pwd)/mylibwrapper

On completion, `scorep-libwrap-init` prints a short-form of how to build and use
the wrapper library to the terminal.

`scorep-libwrap-init` creates a number of files in the working directory:

  - `libwrap.c` just includes `libwrap.h` and should not be changed.
  - `libwrap.h` needs to be edited to contain the headers typically included
    when using the target library.
  - `main.c` will be used to compile and link test programs to make sure the
    target library and wrapper work correctly.
  - `Makefile` can be modified and will be guide you through user library
    wrapper creation workflow.
  - `*.filter` is a Score-P filter file used for excluding/including certain
    files and functions from wrapping. By default it excludes every file except
    the ones in the directories specified via `-I` arguments in the cppflags.
    This means, hopefully, only the target library will be wrapped, and
    sub-headers from, e.g., `/usr/include` will not be wrapped.
  - `README.md` is the file containing this text

Step 2: Add library headers
---------------------------

The next step is to add `#include`-statements to `libwrap.h`.
The included files should be headers that applications using the target
library usually include.

Step 3: Create a simple example application
-------------------------------------------

After this, add some basic calls to the target library to `main.c`.
This file will later be compiled, linked and executed to test whether the
target library and wrapper work correctly.

Step 4: Further configure the build parameters
----------------------------------------------

There are a number of variables at the top of `Makefile` that can be adjusted:

  - `LW_NAME` is the name of the wrapper library (same as the `--name`-argument)
  - `LW_DISPLAY_NAME` is the display name of the wrapper library (same as the
    `--display-name`-argument)
  - `LW_PREFIX` is the installation directory (same as the `--prefix`-argument)
  - `LW_BACKEND` contains the compiler backend
    (same as the `--backend`-argument)
  - `CPPFLAGS` contains the preprocessing flags
    (same as the `--cppflags`-argument)
  - `LDFLAGS` contains the linker flags (same as the `--ldflags`-argument)
  - `LIBS` contains the target library and libraries on which the target
    library depends (same as the `--libs`-argument)

There are more variables that hopefully need no manual adjustment.

Step 5: Build the wrapper
-------------------------

Run

    $ make

Possible errors:

  - Cannot find any function to wrap: Reasons for this can be that there are no
    include statements in libwrap.h or that the filter removes all functions.
    `*.filter` contains some examples on how to use it. Make sure that the
    header files of the target library are included/whitelisted. File names in
    the filter are matched against the file names and paths of headers that the
    preprocessing step of the compiler provides.
  - There is mismatch between functions found in the header files and the
    symbols present in the target library: This means the header file analysis
    found functions that are not present in the library files. Follow the steps
    in the next section and find out whether these symbols ought to be wrapped
    or can be ignored.
  - Incorrect or unexpected wrapper: Enable verbose and make anew via
    `make V=1`. If the LLVM/Clang prints out parsing errors, this likely means
    the preprocessing step executed by your compiler, by default, generates code
    which is of a newer language standard than what Clang's default is.
    You can solve this by explicitely specificying the appropriate standard
    via -x or the `Makefile` variable `LW_LANGUAGE`, e.g., `c++11`.

Possible warnings:

  - Ignoring variadic function: variadic functions can not be wrapped, because
    forwarding ellipsis parameters is not possible in C. The warning describes
    how to adjust `Makefile` in case a `va_list`-version of this function
    (like what vprintf is to printf) exists. Add
    `ellipsis_function:va_list_function` to `LIBWRAP_ELLIPSIS_MAPPING_SYMBOLS`.
  - Ignoring function with unknown argument: C functions having an empty
    parameter list are assumed to have an unknown number of parameters.
    In case this function has zero parameters, you can add it to
    `LIBWRAP_VARIADIC_IS_VOID_SYMBOLS` in `Makefile`.

This step creates a number of files:

  - `main` is main.c linked to the target library. Execute it to make sure
    the executable works.
  - `scorep_libwrap_*.c` is the source of the wrapper library.
  - `libwrap.i` is the preprocessed version of libwrap.h/.c used for analyzing
    and creating the wrapper code.
  - `*.symbols` contains all wrapped symbols, it is only used for `make check`.
  - `libscorep_libwrap_*` is the wrapper library.

If you change `libwrap.h`, `main.c`, `Makefile` or `*.filter` repeat this step.
I.e., execute `make` again. Usually the wrapper creation workflow requires
to run `make`, `make check` and after adjusting the wrapper settings to account
for errors and warnings you again run `make` and `make check`.
It can also take more iterations.

Step 6: Verify the wrapper
--------------------------

Run

    $ make check

For each function found in the header files this step first checks whether
there is a corresponding symbol in the library files. Second, it checks
whether this symbol is also present when linking without the target library,
i.e., is present in the executable or system libraries itself.

A list of functions that have no corresponding symbol in the target library is
provided in the filter file `missing.filter`. If there are symbols in it you
want to wrap, reconsider your wrapper settings, if not add these symbols
to the filter file for this wrapper.

If there are symbols that are also present when linking without the target
library, the file `uncertain.filter`, containing these symbols, is created.
Calls to these functions might not be intercepted. It is up to you to
decide whether this behavior is OK or not. You can, but don't have add some
or all of these symbols to the filter file for this wrapper.

As mentioned in the last paragraph of the previous section, repeat `make`
and `make check` if you adjust your wrapper.

Step 7: Install the wrapper
---------------------------

Once the wrapper builds without errors, you are ready to install it. Run

    $ make install

This step installs the wrapper into the Score-P installation or the directory
specified via `--prefix`, and prints a short description of how to use the
wrapper.

Step 8: Verify the installed wrapper
------------------------------------

Run

    $ make installcheck

Links `main.c` using the Score-P instrumenter into `main_wrapped`.
Execute them, with the given set of values for the environment variables
`SCOREP_LIBWRAP_PATH` and `SCOREP_LIBWRAP_ENABLE`, to make sure they are working.
Executing these applications will create Score-P experiment directories with
measurements of main.c and the wrapped target library. Inspect the experiments
to make sure the wrapper works as expected.

Step 9: Use the wrapper
-----------------------

If you installed the wrapper to somewhere other than the Score-P installation
via the `--prefix`-flag, add the appropriate prefix to `SCOREP_LIBWRAP_PATH`.

    $ export SCOREP_LIBWRAP_PATH=$SCOREP_LIBWRAP_PATH:<prefix>

You can then instruct Score-P to use this wrapper by setting the configuration
variable `SCOREP_LIBWRAP_ENABLE`. Which is a list of library wrapper names (the
`--name`-flag) or a full path to the plug-in. The list is delimited by
`SCOREP_LIBWRAP_ENABLE_SEP`. There is no need to recompile or relink the
application.

Example of executing the application:

    $ export SCOREP_LIBWRAP_ENABLE=mylib,myotherlib
    $ ./main

The usual filtering via Score-P's `SCOREP_FILTERING_FILE` applies. A function
that matches the filter is not enabled at all. This provides an overhead-free
runtime filter.

To find out which user library wrappers are installed call

    $ scorep-info libwrap-summary

It lists all found wrappers, either installed into Score-P's installation
directory or found via the `SCOREP_LIBWRAP_PATH` environment variable.
Optionally you can run

    $ scorep-info libwrap-summary <wrappername>

to show the configuration of a specific wrapper.

Workflow in short
=================

    $ scorep-libwrap-init --name=<name> -x c --cppflags="<>" --ldflags="<>" \
          --libs="<>" <workingdir>
    $ cd <workingdir>
    # Add headers to libwrap.h
    # Add example to main.c
    $ make
    $ make check
    # Remove errors by adjusting the wrapper settings and adding symbols to
    # filter
    # Repeat make && make check until all errors are solved
    $ make install
    $ make installcheck
    $ export SCOREP_LIBWRAP_ENABLE=<name>
    $ ./myprogram
    # Inspect the experiment directory scorep-*

Implementation details
======================

Score-P user library wrapping enables users to intercept and analyse any
C or C++ library. For this we rely on *libclang* to produce a list of functions
from the header files of the target library. The wrapper itself relies on
symbol-based wrapping supplied by the linker and dynamic linker.

Many C++-libraries rely heavily on inlining and templates.
Wrapping libraries based on symbols being present in the target library means
that this technique is unable to intercept any inlined function calls.

libclang's ability to detect to-be-inlined functions in header files
improved with LLVM 3.9. For LLVM versions <= 3.8 the library wrapper generator
will likely generate function wrappers for functions that are inlined and
cannot be wrapped. The workflow will provide you with a warning and what
to do in this case.

Miscellaneous
=============

If you are lost, run:

    $ make help

This command displays how the wrapper is currently configured:

    $ make show-summary

Use this information to, e.g., redo the wrapper, or update the wrapper files via
`--update` if, e.g., there is new Score-P version.

FAQ
===

  - Open issues can be found in Score-P's `OPEN_ISSUES` file under
    "User library wrapping"
