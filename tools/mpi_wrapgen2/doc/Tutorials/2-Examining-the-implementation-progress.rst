.. _examining-the-implementation-progress:

2. Examining the Implementation Progress
========================================

The MPI Standard specifies more than 400 :term:`procedures<procedure>`,
each in multiple :term:`bindings<binding>`. With each new version, more
procedures are added. Therefore, the ``mpi_wrapgen2`` tool provides
functionality to keep an overview on the current implementation status.

Driver Output
-------------

Call the driver script that is responsible for producing raw source files from
templates:

.. code-block:: sh

    ./driver.py

.. note::

    The main :program:`mpi_wrapgen2.sh` script internally calls the driver, but
    additionally beautifies the output and installs the results into the source
    directory of Score-P.

At the end of the output, the driver displays a progress statistic similar to
this:

::

    INFO: Excluded 57 of the 491 procedures in the MPI Standard.
    INFO: Progress statistics by layer:
       layer   not seen       todo       done        all
    ----------------------------------------------------
           C        433          0          0        433
           F        407          0          0        407
         F08          1          4        388        393

All procedures that do not have an entry in :file:`scorep.json` are excluded
from being wrapped. The 'all' column in the statistic refers to all procedures
that have a binding in the respective language and were not excluded.

.. note::

    Normal and large-count (embiggened) procedures are not counted separately.
    E.g. ``MPI_Send`` and ``MPI_Send_c`` are considered the same procedure by
    the generator.

The column 'not seen' counts procedures for which none of the :term:`template`
files in ``templates/`` contains a directive to generate them. In contrast, there
exist directives to generate the 'todo' procedures but the resulting wrapper might
miss some features. The 'done' column counts all procedures for which wrappers
are generated and no todos are known.

.. note::

    The 'done' status only means that the generator does not know of any 'todos'
    for this procedure. Maintaining the 'todos' is (mostly) the user's
    responsibility. See :ref:`section_common_tasks`.


.. warning::

    Any warnings reported by the driver, for example

    ::

        WARNING: Some parameters are currently not handled correctly by the Fortran wrapper layer.
        No Fortran wrapper for mpi_alltoallw is generated

    should be investigated and fixed before committing a change.


Driver Report
-------------

The generator driver also reports the progress for each procedure and layer in
a separate file. Start a Python console and open the report:

.. code-block:: python

    import json
    with open('report', 'r') as file:
         report = json.load(file)

The report is now available as a Python dictionary which can be queried for
detailed information on the status of the generated wrappers. The summary
listed 4 procedures as 'not seen' and one as 'todo' and you should find out the
names:

.. code-block:: python

    # F08 procedures where the generated wrapper has a 'todo'
    report['F08']['todo']

.. code-block:: python

    # F08 procedures for which no wrapper has been generated
    report['F08']['not seen']


Other possible uses of this dictionary are:

- Check whether an F08 wrapper for ``Mpi_Info_get_nkeys`` has been generated:

.. code-block:: python

    # Note that 'mpi_info_get_nkeys' is all lowercase.
    'mpi_info_get_nkeys' in  report['F08']['todo'] + report['F08']['done']

- List all procedures that are 'done' in C, but not in Fortran 2008:

.. code-block:: python

    set(report['C']['done']).difference(set(report['F08']['done']))

- List all excluded procedures:

.. code-block:: python

    report['excluded']
