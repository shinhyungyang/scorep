1. Generating and Installing the Wrappers
=========================================

MPI wrappers are located in the MPI adapter directory (``../../src/adapters/mpi``)
of the Score-P sources and grouped by language binding. List source files for
the Fortran 2008 bindings

.. code-block:: sh

    ls  ../../src/adapters/mpi/f08/wrappers

which shows

::

    SCOREP_Mpi_F08_Cg.F90
    SCOREP_Mpi_F08_Coll.F90
    SCOREP_Mpi_F08_Env.F90
    ...

Sources for the C bindings are placed in ``../../src/adapter/c/mpi``.
Now, delete all F08 wrappers:

.. code-block:: sh

    rm ../../src/adapters/mpi/f08/wrappers/*

Deleting the wrappers is not an issue, as restoring them can be done by
re-running the wrapper generator. For this, simply call the main script:

.. code-block:: sh

    ./mpi_wrapgen2.sh

The output of :program:`mpi_wrapgen2.sh` should be similar to this:

::

    Ensuring the output directory 'output' is clean
    Generating outputs
    INFO: Initializing wrapgen
    INFO: Loading Score-P data from file tools/mpi_wrapgen2/generator/data/scorep/scorep.json.
    ...
    INFO: Selected template subdirs: ('c', 'f08', 'm4', 'adapter')
    INFO: Generating from templates in 'templates/c'
    INFO: Generating output from 'templates/SCOREP_Mpi_C_Cg.c.tmpl'
    ...
    INFO: Generating from templates in 'templates/f08'
    INFO: Generating 'output/f08/SCOREP_Mpi_F08_Cg.F90' from 'templates/f08/SCOREP_Mpi_F08_Cg.F90.tmpl'
    ...
    INFO: Excluded 57 of the 491 procedures in the MPI Standard.
    INFO: Progress statistics by layer:
    INFO: Progress statistics by layer:
       layer   not seen       todo       done        all
    ----------------------------------------------------
           C        433          0          0        433
           F        407          0          0        407
         F08          0          0        393        393
    WARNING: Found 1 todos and 3 warnings.
    Beautifying outputs
    BEAUTIFY output/f08/SCOREP_Mpi_F08_Cg.F90
    ...
    Installing generated files
    Updating file ../../src/adapters/mpi/f08/wrappers/SCOREP_Mpi_F08_Cg.F90
    ...
    Successfully generated and updated wrappers


The script always cleans the output directory ``output/`` before generating the
wrappers. During initialization the generator reads input data from the files in
``generator/data``. Then, it generates an output file for each template in
``templates/``. After all files have been generated, the script prints a summary
showing how many wrappers have been generated for each layer. More detail on
this summary is shown in the :ref:`next tutorial<examining-the-implementation-progress>`.
The generated files are then beautified to match Score-P's code style.
Finally, the files in ``output/`` which differ from their installed counterparts
are installed into the Score-P source code. Now, the contents of the
``wrappers`` directory should be restored.

Call the script a second time. This time, nothing is installed,
because there was no change in the output files:

::

    ...
    Installing generated files
    Successfully generated and updated wrappers
