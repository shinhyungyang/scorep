5. Writing a Task for Multiple Wrappers
=======================================

In the last tutorial you implemented a specialized task for the
``MPI_Info_get_nkeys`` procedure. In this tutorial you will write one task and
apply it to multiple wrappers.

The new task will detect changes in the number of info keys in procedures that
take an info argument. Ideally, it will be applicable to all such procedures.


Parametrized Output
-------------------

Add a new task:

.. code-block:: python
   :caption: wrapgen/generator/f08/wrapper_tasks/common.py
   :emphasize-lines: 3-4

     class TaskDetectInfoNkeysChange(F08Task):

        def needs_parameter_names(self) -> Scope[str]:
            return self.make_parameter_name_mapping('info')

        def generate_comment(self) -> GeneratorOutput:
            yield f"!> Detects changes in the number of keys in {self.ipn.info} and prints those to the console\n"

Don't forget to add ``'TaskDetectInfoNkeysChange'`` to the ``__all__`` list.
The visible effect of this task is to insert a comment. The content of that
comment, however, depends on the ``ipn`` property (short for 'internal parameter
names') of the task. This object is constructed by the highlighted
``needs_parameter_names`` function. At the moment it has the single property 'info'
with the value 'info'.

This might seem somewhat redundant at the moment. One immediate benefit of using
the ``needs_parameter_names`` function is that it adds validation. Try applying the
task to the ``MPI_Send`` procedure, which does not have a parameter 'info', and
then call the main script. It should report an error:

::

    KeyError: "No parameter named 'info' in 'MPI_Send'"


Updating 'scorep.json'
----------------------

To find out which procedures in the MPI Standard take an argument named 'info',
run the following code block in a Python console:

.. code-block:: python

   import wrapgen.data.mpistandard as std

   std.init()


   def has_info_parameter(proc: std.F08Symbol):
       for param in proc.parameters:
           if param.name == 'info' and param.kind.name == 'INFO':
               return True
       return False


   std_f08_procs_with_info = [
       name
       for name, proc in std.PROCEDURES.items()
       if proc.express.f08 is not None
          and has_info_parameter(proc.express.f08)
          and not proc.name.startswith('mpi_info_create')
          and not proc.name.startswith('mpi_info_delete')
   ]

There are over 60 procedures in that list. Updating the scorep.json file for
each of them is tedious and error-prone. It is better to update the data
programmatically via a call to :func:`wrapgen.data.scorep.update_json_data`.
In the same console, execute:

.. code-block:: python
   :emphasize-lines: 12

   import wrapgen.data.scorep as scp

   def my_update(scorep_data):
       for p in filter(lambda x: x in scorep_data, std_f08_procs_with_info):
           if 'tasks' not in scorep_data[p]:
               scorep_data[p]['tasks'] = list()
           scorep_data[p]['tasks'] = [t for t in scorep_data[p]['tasks'] if t['typename'] != 'TaskDefault']
           scorep_data[p]['tasks'].append({"typename": "TaskDetectInfoNkeysChange"})
           print(f"Appended TaskDetectInfoNkeysChange to '{p}'")
       return scorep_data

   scp.update_json_data(update_function=my_update)

The new task is now added to all the functions in the list. Call the main
:program:`mpi_wrapgen2.sh` script and observe the changes.


Local Variables
---------------

The wrapper has to determine the number of keys in the info object before and
after the call to the wrapped procedure. To get the number before, add these
two methods the task:

.. code-block:: python
   :caption: wrapgen/generator/f08/wrapper_tasks/common.py

    def needs_local_variables(self) -> Dict[str, FortranVariable]:
        return Scope.from_named_items(
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_nkeys_before',
                    type_name='integer'),
                initial_value='0'),
            FortranVariable(
                parameter=FortranArgument.new(
                    name='local_ierror',
                    type_name='integer')))

    def generate_pre_pmpi_call(self) -> GeneratorOutput:
        yield f"call PMPI_Info_get_nkeys({self.ipn.info}, {self.ln.local_nkeys_before}, {self.ln.local_ierror})\n"

The method :meth:`generate_pre_pmpi_call` adds the code that queries the info
object for the number of keys and stores the result in the local variable
``local_nkeys_before``. It makes use of the ``ln`` :class:`Scope` for local names
which is filled by :meth:`needs_local_variables`.

Tasks manage the declaration and initialization of local variables. In Fortran,
this is especially important, because non-executable statements, e.g. variable
declarations, come before executable statements. In :meth:`needs_local_variables`
one can add declarations and (optionally) initializations for the local
variables. In this case, the ``local_nkeys_before`` variable of type ``integer``
is declared, and initialized to '0'. The integer variable ``local_ierror`` is
declared, but not initialized.

The 'local\_' prefix to the variable names is not necessary, but it avoids
accidental name collisions with other parameters.

.. warning::

    If multiple tasks declare a local variable with the same name, it is only
    declared and initialized once.


Final Implementation
--------------------

It is time to implement the intended functionality of the task: to detect
changes in the number of keys in the 'info' parameter. A possible implementation
looks like this:

.. code-block:: python
   :caption: wrapgen/generator/f08/wrapper_tasks/common.py

   class TaskDetectInfoNkeysChange(F08Task):
       def needs_parameter_names(self) -> Scope[str]:
           return self.make_parameter_name_mapping('info', 'ierror')

       def generate_comment(self) -> GeneratorOutput:
           yield f"!> Detects changes in the number of keys in {self.ipn.info} and prints those to the console\n"

       def needs_local_variables(self) -> Dict[str, FortranVariable]:
           return Scope.from_named_items(
               FortranVariable(
                   parameter=FortranArgument.new(
                       name='local_nkeys_before',
                       type_name='integer'),
                   initial_value='0'),
               FortranVariable(
                   parameter=FortranArgument.new(
                       name='local_nkeys_after',
                       type_name='integer'),
                   initial_value='0'),
               FortranVariable(
                   parameter=FortranArgument.new(
                       name='local_ierror',
                       type_name='integer')))

       def get_nkeys(self, store_into):
           return f"call PMPI_Info_get_nkeys({self.ipn.info}, {store_into}, {self.ln.local_ierror})\n"

       def generate_pre_pmpi_call(self) -> GeneratorOutput:
           yield self.get_nkeys(self.ln.local_nkeys_before)

       def generate_post_pmpi_call(self) -> GeneratorOutput:
           yield self.get_nkeys(self.ln.local_nkeys_after)
           yield f"""\
   if ({self.ln.local_nkeys_before} .ne. {self.ln.local_nkeys_after}) then
       print '(a, " changed the number of keys in ", a, " from ", i2, " to ", i2)', '{self.scp_procedure.std_name}', '{self.ipn.info}', {self.ln.local_nkeys_before}, {self.ln.local_nkeys_after}
   else
       print '(a," did not change the number of keys in ", a)', '{self.scp_procedure.std_name}', '{self.ipn.info}'
   end if
   """

Regenerate the wrappers and recompile Score-P. Then try it out on the test
program :download:`mpi-info.f90`:

.. code-block:: sh

   scorep-mpif90 -o mpi-info mpi-info.f90
   ./mpi-info


This should produce the following output:

::

   MPI_Info_set changed the number of keys in info from  0 to  1
   MPI_Info_set changed the number of keys in info from  1 to  2
   MPI_Info_set did not change the number of keys in info
   MPI_Info_get_valuelen did not change the number of keys in info
   MPI_Info_get did not change the number of keys in info
   Info[key1] = Congratulations: You finished the tutorial!


Congratulations, you have completed the tutorial successfully! By now, you know
how to invoke ``mpi_wrapgen2`` to regenerate all files, how to interpret its
output, and how to new wrappers and customize their code.

For details on the inner workings of the generator, please refer to the
:ref:`section_reference`.
