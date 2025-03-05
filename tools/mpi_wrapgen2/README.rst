MPI Wrapper Generator: 2nd Generation
=====================================

.. note:: A.k.a. mpi_wrapgen2, w2g.

This tool generates Score-P's wrapper code around the MPI library.
``mpi_wrapgen2`` is intended as a replacement of ``mpi_wrapgen`` with these
goals in mind:

* Cover all MPI procedures
    * in all versions of the MPI standard,
    * in all language bindings (C, Fortran 90, Fortran 08),
    * with and without embiggening.
* Work with the common MPI implementations. At least
    * MPICH and derivatives (e.g. ParaStation MPI)
    * Open MPI
* Derive as much information from the MPI Standard as possible.
* Automatically integrate the generated sources into Score-P.
* Provide information on coverage.

.. _section_dependencies:

Dependencies
------------

The generator is a tool for Score-P developers, and will not be distributed as
part of the tarballs. No dependencies will be added for Score-P users.

The generator is written in `Python3`_ ( > 3.10 ). It depends on

* `pympistandard`_ as an API to the MPI Standard,
* `Jinja2`_ to render the source files from templates,
*  and `Sphinx`_ to build documentation for this project.

Install the requirements into a virtual environment, via

.. code-block:: sh

    python3 -m venv .venv
    source .venv/bin/activate
    python3 -m pip install -r requirements.txt

The above is enough to produce functional, but ugly wrapper files using the
:term:`driver.py` script. The automatic installation of generated files to the
Score-P source tree includes an additional *formatting* step, relying on
`uncrustify`_ for C files and `fprettify`_ for Fortran files. The main
``mpi_wrapgen2.sh`` script assumes that these tools are installed. Both are
provided by recent versions of `perftools-dev`_.

.. _Python3: https://docs.python.org/3.10/
.. _pympistandard: https://github.com/mpi-forum/pympistandard
.. _Jinja2: https://jinja.palletsprojects.com/en/3.1.x/
.. _Sphinx: https://www.sphinx-doc.org/en/master/index.html
.. _uncrustify: https://github.com/uncrustify/uncrustify
.. _fprettify: https://github.com/pseewald/fprettify
.. _perftools-dev: https://perftools.pages.jsc.fz-juelich.de/utils/perftools-dev/


Quick Start
-----------

The main script generates and beautifies all wrapper sources, and installs them
into the source tree of Score-P:

.. code-block:: sh

    ./mpi_wrapgen2.sh


To build the in-depth documentation for this project, run

.. code-block:: sh

    cd doc
    make html


Project Structure
-----------------

.. glossary::
    mpi_wrapgen2.sh
        This is the main script that does everything:

        - Calls the :term:`driver.py` script to generate wrapper files,
        - beautifies the generated files with ``uncrustify`` or ``fprettify``,
        - compares the resulting files (in :term:`output/`) with the file present
          in Score-P, and
        - installs all added or changed files into Score-P.

    driver.py
        Generates wrapper sources from template files:

        - Generates a source file in :term:`output/` for each template file in
          :term:`templates/`,
        - calls the code in :term:`wrapgen/` to generate wrapper code, and
        - writes a report in json format that lists the generated wrappers.

    wrapgen/
        Python module which implements the generator logic.

    templates/
        Template files with ``.tmpl`` extension to generate source files from.

    output/
        Directory for output of the :term:`driver.py` script.

    doc/
        In-depth documentation.


.. This files serves also as the index.rst file for the Sphinx documentation in doc/

More Documentation
------------------

.. toctree::
    :maxdepth: 3

    Tutorials/index
    Reference/index
    glossary


Indices and Tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
