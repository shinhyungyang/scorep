Glossary
========

.. glossary::
    :sorted:

    procedure
        Is defined in several contexts:

        - MPI: A (language independent) procedure as defined by the MPI Standard.
          It can have one or more language bindings. A language binding can
          declare one or more function|procedure|subroutine prototypes. For
          example, the ``MPI_SEND`` procedure has

            - a C-binding with different function prototypes for normal
              (``MPI_Send``) and large-count(``MPI_Send_c``) arguments
            - a Fortran binding with the ``MPI_SEND`` subroutine
            - a Fortran 2008 binding with interface ``MPI_SEND`` for default and
              large-count variants
            - a language independent specification (LIS)

        - pympistandard: An instance of the ``Procedure`` class or one of its
          subclasses in the ``pympistandard`` module. This is closely related to
          the MPI procedure, because the ``pympistandard`` is designed to
          provide a Python interface to the machine-readable API specification
          of MPI.
        - generator: An instance of the ``ScorepProcedure`` class or one of its
          subclasses in the ``generator`` module. It contains a pympistandard
          procedure and provides additional query functions.
        - Fortran: A Fortran procedure as defined by the Fortran language
          standard. Can be either a subroutine or a function.

    function
        See also :term:`procedure` (Fortran).

        In Fortran, a function is a procedure that returns a value (As opposed
        to a subroutine, which does not return anything.)

    expression
        See :term:`procedure` (pympistandard).

        In the :term:`pympistandard` tool, a procedure provides expressions for
        each prototype in each *language binding* defined by the *MPI procedure*.
        An *expression* is a view into a specific prototype.

    prototype
        Can be one of those things:

        - MPI: Calling convention (name and argument list) for one concrete
          function/subroutine of the MPI Standard
        - legacy ``mpi_wrapgen``: An entry in the ``SCOREP_Mpi_Prototypes.xml``
          file

    template
        A file which contains placeholders to be filled in by a template engine.

        - ``mpi_wrapgen2``: Always a file with ``.tmpl`` ending. Uses Jinja2 as
          template engine.
        - legacy ``mpi_wrapgen``: The legacy wrapper generator distinguishes
          *file-level templates* (``.tmpl`` ending) and *function-level templates*
          (``.w`` ending). It provides its own template engine.

    generator
        Python module that produces source files for *Score-P* from
        :term:`template`. It combines information from the MPI machine-readable
        API file, using :term:`pympistandard`, and *Score-P* .

    layer
        The :term:`generator` generates various parts of the Score-P code base.
        It distinguishes four layers:

        1. The C-bindings for MPI procedures
        2. The Fortran-bindings for MPI procedures
        3. The F08-bindings for MPI procedures
        4. The autoconf checks related to MPI procedures

        The first three layers correspond to the MPI language bindings :term:`binding`

    task
        Tasks can be added to a :term:`procedure` to add code to the generated
        wrapper. A procedure can have an arbitrary number of tasks.
        Tasks can be orthogonal, but some tasks may depend on others.

    binding
        An interface for an MPI :term:`procedure`. MPI defines three sets of
        bindings for the languages C, Fortran, and Fortran 2008. A procedure
        has interfaces in one or more language.

    pympistandard
        Python module to access the data in the MPI machine readable API file.
        See also :term:`apis.json`.

    apis.json
        Since version 4.0 the MPI standard provides API information for third-
        party tools in a machine readable format via this file.

    scorep.json
        Contains additional information to produce the Score-P wrappers.
