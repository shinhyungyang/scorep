.. _adding-a-new-wrapper:

3. Adding a New Wrapper
=======================

In this tutorial, you will add a Fortran 2008 wrapper for ``MPI_Info_get_nkeys``:

.. code-block:: Fortran

    MPI_Info_get_nkeys(info, nkeys, ierror)
        TYPE(MPI_Info), INTENT(IN) :: info
        INTEGER, INTENT(OUT) :: nkeys
        INTEGER, OPTIONAL, INTENT(OUT) :: ierror

Open the file :file:`templates/f08/SCOREP_Mpi_F08_Misc.F90.tmpl` and look for
this part:

.. code-block:: python
    :caption: templates/f08/SCOREP_Mpi_F08_Misc.F90.tmpl
    :emphasize-lines: 5

    {% set procedures = [
    'MPI_Alloc_mem',
    ...
    'MPI_Info_get',

    'MPI_Info_get_nthkey',
    ...
    'MPI_Status_f2f08',
    ] %}
    ...
    {{ wrapgen.generate_for_procedures(procedures, wrapgen.callbacks.generator_callback_f08_wrapper) }}


This Jinja2 directive calls a function of the :py:mod:`wrapgen` module to
generate a list of MPI wrappers. Add ``'MPI_Info_get_nkeys'`` to the list of
procedures:

.. code-block:: python
    :caption: templates/f08/SCOREP_Mpi_F08_Misc.F90.tmpl
    :emphasize-lines: 5

    {% set procedures = [
    'MPI_Alloc_mem',
    ...
    'MPI_Info_get',
    'MPI_Info_get_nkeys',
    'MPI_Info_get_nthkey',
    ...
    'MPI_Status_f2f08',
    ] %}

Try to call the main :program:`mpi_wrapgen2.sh`. It fails with an exception!

::

    RuntimeError: Requested to generate from 'generator_callback_f08_wrapper'
    for 'MPI_Info_get_nkeys', but there is no entry for that name in scorep.json

The file :file:`scorep.json` contains all Score-P related information on MPI
procedures. Procedures not listed in this file are excluded from being wrapped,
and requesting to wrap them is an error.

.. note::
    Passing the ``--permissive`` option to the driver, e.g.

    .. code-block:: sh

        ./mpi_wrapgen2.sh --driver-args --permissive

    causes the driver to continue with a warning on such recoverable errors. This is
    useful during development.

To fix the error, add a new entry for 'MPI_Info_get_nkeys' to :file:`scorep.json`.
Note the lowercase spelling!

.. code-block:: python
    :caption: generator/data/scorep/scorep.json

    "mpi_info_get_nkeys": {
        "mpi_version": "2.0"
    }

Call the main script again. This time, it reports:

.. code-block:: output
    :emphasize-lines: 4

    Installing generated files
    Updating file ../../src/adapters/mpi/c/wrappers/SCOREP_Mpi_C_Reg.c
    Updating file ../../src/adapters/mpi/c/wrappers/SCOREP_Mpi_C_Reg.h
    Updating file ../../src/adapters/mpi/f08/wrappers/SCOREP_Mpi_F08_Misc.F90
    Updating file ../../src/adapters/mpi/f08/wrappers/SCOREP_Mpi_F08_Reg.F90
    Updating file ../../build-config/m4/scorep_mpi_c_decls.m4
    Updating file ../../build-config/m4/scorep_mpi_f08_symbols.m4
    Successfully generated and updated wrappers

In addition to producing a wrapper, the generator updates various other files
where a list of the wrapped functions needs to be maintained. The newly produced
wrapper should look like this:

.. code-block:: Fortran
    :caption: <scorep>/src/adapters/mpi/f08/wrappers/SCOREP_Mpi_F08_Misc.F90
    :emphasize-lines: 5

    !>
    !> Measurement wrapper for MPI_Info_get_nkeys in the Fortran 2008 bindings.
    !> @note Introduced in MPI 3.0
    !> @ingroup MISC
    !> TODO: Implement more than the default behavior for mpi_info_get_nkeys
    !>
    #if defined (SCOREP_F08_SYMBOL_NAME_MPI_INFO_GET_NKEYS)

    #if ( defined( HAVE_F08_TS_BUFFERS_MPI_INFO_GET_NKEYS ) && HAVE_F08_TS_BUFFERS_MPI_INFO_GET_NKEYS )
    #define CHOICE_BUFFER_TYPE type(*), dimension(..)
    #else
    #define CHOICE_BUFFER_TYPE type(*), dimension(*)
    #endif
    subroutine SCOREP_F08_SYMBOL_NAME_MPI_INFO_GET_NKEYS( &
        info, &
        nkeys, &
        ierror)

        use :: scorep_mpi_f08
        use :: mpi_f08, only: &
            MPI_INFO, &
            PMPI_Info_get_nkeys

        implicit none
        ...

By default, the generator produces a minimal but fully functional wrapper from
the information the MPI Standard provides. This wrapper records an ``Enter`` and
and ``Exit`` event. In the next tutorial you will learn to customize the
generated wrapper code.

The default wrappers are marked with a TODO, and are listed as 'todo'
in the progress report of :program:`driver.py`. The driver should now report
'MPI_Info_get_nkeys' as 'todo'.
