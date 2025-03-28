Tutorials
=========

This collection of tutorials guide through the usage of the ``mpi_wrapgen2``
tool, including adding additional wrappers, generating them and inspecting the
results.

Before you continue, please make sure that the dependencies for
``mpi_wrapgen2`` are installed as in see :ref:`section_dependencies`.

Each tutorial assumes that the repository is in a well-defined state at the
beginning. To set the stage for tutorial ``<number>``, use the helper
script:

.. code-block:: sh

    # Omit <number> to start at the beginning
    ./doc/wrapgen_tutorial.sh begin <number>


Once you want to finish the tutorial, you can restore a clean repository with

.. code-block:: sh

    ./doc/wrapgen_tutorial.sh end


.. note::

    It is recommended to work through the tutorials in order.

.. toctree::
    :maxdepth: 1

    1-Generating-and-installing-the-wrappers
    2-Examining-the-implementation-progress
    3-Adding-a-new-wrapper
    4-Customizing-a-single-wrapper
    5-Writing-a-task-for-multiple-wrappers
