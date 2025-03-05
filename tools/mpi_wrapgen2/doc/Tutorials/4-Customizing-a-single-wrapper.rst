4. Customizing a Single Wrapper
===============================

In the :ref:`last tutorial <adding-a-new-wrapper>` you have added a default
wrapper which records ``Enter`` and ``Exit`` events. Most wrappers have to do
more though, for example point-to-point communication functions also record
``MPI_Send`` or ``MPI_Recv`` events. In this tutorial you will learn to
customize a wrapper. Therefore, you will modify the Fortran 2008 wrapper for
``MPI_Info_get_nkeys`` to print out the number of keys in the info object.

Score-P Related Data
--------------------
The module :py:mod:`generator.data.scorep` provides data related to Score-P,
which is of course not part of the MPI Standard.

Start a Python console and type these commands

.. code-block:: python

    import wrapgen.data.scorep as scp
    scp.init()

to load and initialize the module. Print out the data for
``MPI_Info_get_nkeys``:

.. code-block:: python

    scp.SCOREP_GENOME['info_get_nkeys']


It should be similar to:

::

   Genes(mpi_version=2.0, large_mpi_version=0.0, group='misc', guards=['extra', 'misc'], region=<RegionType.none: 'none'>, parameters={}, template='DefaultWrapper', c_variants=[], tasks=[TaskData(typename='TaskDefault', attributes={})])


The procedure was introduced in version 2.0 of the MPI Standard. In Score-P it
belongs to the 'misc' group and has include guards for groups 'extra' and
'misc'. ``parameters`` is empty, so nothing special is done for the function
parameters. The wrapper will use the default template and a default
:term:`task`.

This data is stored in the file :file:`generator/data/scorep/scorep.json`.
Locate the entry for 'MPI_Info_get_nkeys' by searching for 'mpi_info_get_nkeys':

..  code-block:: json
    :caption: generator/data/scorep/scorep.json

    "mpi_info_get_nkeys": {
        "group": "misc",
        "guards": [
            "extra",
            "misc"
        ],
        "mpi_version": "2.0",
        "tasks": [
            {
                "typename": "TaskDefault"
            }
        ]
    },


Adding a Task
-------------

Extra generated content is controlled by so-called :term:`tasks<task>`. The task
``TaskDefault`` signals that the default behavior is enough and suppresses the
'todo' warning. It has to be the only task in a procedure. Now we are going to
change the wrapper. Remove the entire entry for ``TaskDefault``. Then, add
another task to :file:`scorep.json`:

.. code-block:: json
    :caption: generator/data/scorep/scorep.json
    :emphasize-lines: 9-14

    "mpi_info_get_nkeys": {
        "group": "misc",
        "guards": [
            "extra",
            "misc"
        ],
        "mpi_version": "2.0",
        "tasks": [
            {
                "typename": "TaskTodo",
                "attributes":  {
                    "message": "Print the number of keys to the console."
                }
            }
        ]
    },

The task ``TaskTodo`` unsurprisingly adds a 'todo' comment to the wrapper. Run the
main script and view the wrapper code:

.. code-block:: Fortran
    :caption: SCOREP_Mpi_F08_Misc.F90
    :emphasize-lines: 5

    !>
    !> Measurement wrapper for MPI_Info_get_nkeys in the Fortran 2008 bindings.
    !> @note Introduced in MPI 2.0
    !> @ingroup misc
    !> TODO: Print the number of keys to the console.
    !>
    ...
    subroutine SCOREP_F08_SYMBOL_NAME_MPI_INFO_GET_NKEYS( &

As long as a ``TaskTodo`` typename is present for an MPI procedure, it will be
listed in the 'todo' category of the progress report.


Listing Tasks
-------------

Now is a good time to check which tasks are already implemented. The
:py:mod:`wrapgen.generator` module provides a helper function for that. In a
Python console, execute

.. code-block:: python

    from wrapgen.generator import print_tasks_in_layer, Layer
    print_tasks_in_layer(Layer.F08)

to list all tasks for the Fortran 2008 wrappers.

.. note::

    A separate set of tasks is implemented for the C wrappers in the 'C' layer.

In the long list of tasks, there is an entry for TaskTodo:

.. code-block::
    :emphasize-lines: 6

    Task: TaskTodo
    --------------------------------------------------------------------------------
        Dependencies:
        Local variables:
        Hooks:
            generate_comment(from TaskTodo)
    --------------------------------------------------------------------------------

For each of the tasks, the list shows its dependencies on other tasks, the
local variables that it will add to the wrapper code, and a list of hooks.
A hook is a method with a specific name that will generate code in the wrapper.
``TaskTodo`` implements the :py:func:`generate_comment` hook, which generates
content in the Doxygen comment above the actual function definition.


Implementing a Simple Task
--------------------------

There is no task that produces a print statement, so you have to create a new
one. Although it would be better to group MPI_Info related tasks in their own
submodule eventually, we are focusing on implementing one simple task only, and
hence implement it in the :py:mod:`wrapgen.generator.f08.wrapper_tasks.common`
module.

Add the following code to the module

.. code-block:: python
    :caption: wrapgen/generator/f08/wrapper_tasks/common.py
    :emphasize-lines: 5-6

    class TaskPrintInfoNkeys(F08Task):
        def generate_comment(self) -> GeneratorOutput:
            yield "!> Prints the number of keys to the console\n"

        def generate_post_pmpi_call(self) -> GeneratorOutput:
            yield "print '(a, i2)', 'nkeys = ', nkeys\n"

All tasks in the F08-layer should derive from :py:class:`F08Task`. The new task
implements two hooks: The :py:func:`generate_comment` hook you have seen before,
and the :py:func:`generate_post_pmpi_call` hook which will generate a print
statement right after the call to the wrapped function.

To make the new task visible to the parent module, add ``'TaskPrintInfoNkeys'``
to the ``__all__`` list in the module.

To make the wrapper for ``MPI_Info_get_nkeys`` use the task, add it to the task
list in :file:`scorep.json`:

.. code-block:: json
    :caption: wrapgen/data/scorep/scorep.json
    :emphasize-lines: 7-10

    "tasks": [
        {
            "typename": "TaskTodo",
            "attributes":  {
                "message": "Print the number of keys to the console."
            }
        },
        {
            "typename": "TaskPrintInfoNkeys"
        }
    ]

Run the main script and view the generated code:

.. code-block:: Fortran
    :caption: SCOREP_Mpi_F08_Misc.F90
    :emphasize-lines: 7

    call PMPI_Info_get_nkeys( &
        info, &
        nkeys, &
        internal_ierror)
    call scorep_set_in_measurement(scorep_in_measurement_save)

    print '(a, i2)', 'nkeys = ', nkeys
    ...

As expected, a print statement has been inserted after the call to the wrapped
procedure. The todo comment is still there. If more than one task implements
the same hook, output from both is added in the order that the tasks are given in
:file:`scorep.json`. It is time to remove that comment though. Simply remove the
task from the list and regenerate the files.
