import copy

from wrapgen.generator.scorep_parameter import ScorepParameter
from wrapgen.generator.wrapper_hooks import GeneratorOutput
from wrapgen.data import mpistandard as std
from wrapgen.language.fortran import FortranArgument
from wrapgen.language.pympistandard_to_f import make_fortran_argument


class ScorepF08Parameter(ScorepParameter):
    def __init__(self, std_parameter: std.F08ParameterBase, **kwargs):
        super(ScorepF08Parameter, self).__init__(**kwargs)
        self._std_parameter = std_parameter
        self._parameter_info: FortranArgument = make_fortran_argument(std_parameter)

    def _get_std_parameter(self) -> std.F08ParameterBase:
        return self._std_parameter

    @property
    def parameter(self) -> FortranArgument:
        return self._parameter_info

    def internal_parameter(self) -> FortranArgument:
        internal_info = copy.deepcopy(self._parameter_info)
        if self.has_internal_replacement():
            # intent and optional is not allowed for local variables
            internal_info.name = f'internal_{self._parameter_info.name}'
            internal_info.descriptor.intent = None
            internal_info.descriptor.attributes = list(filter(
                lambda x: x != 'OPTIONAL',
                self._parameter_info.descriptor.attributes))
            if self._handle_status_ignore():
                assert self.is_status()
                internal_info.descriptor.attributes.append('POINTER')
            if self._handle_statuses_ignore():
                assert self.is_status_array()
                internal_info.descriptor.dimension = [f':']
                internal_info.descriptor.attributes.append('POINTER')
        return internal_info

    def has_internal_replacement(self) -> bool:
        return self.is_optional_out() \
            or self._handle_status_ignore() \
            or self._handle_statuses_ignore()

    def generate_init(self) -> GeneratorOutput:
        if self._handle_status_ignore():
            yield f'''\
if ( scorep_mpi_is_status_ignore({self._std_parameter.name}) ) then
    call c_f_pointer( scorep_mpi_get_status_array(1_c_size_t), {self.internal_parameter().name} )
else
    {self.internal_parameter().name} => {self._std_parameter.name}
end if
'''
        if self._handle_statuses_ignore():
            yield f'''\
if ( scorep_mpi_is_statuses_ignore({self._std_parameter.name}) ) then
    call c_f_pointer( scorep_mpi_get_status_array(int({self._status_array_length()},c_size_t)), &
                     {self.internal_parameter().name}, &
                     shape=[{self._status_array_length()}] )
else
    {self.internal_parameter().name} => {self._std_parameter.name}(:{self._status_array_length()})
end if
'''

    def generate_cleanup(self) -> GeneratorOutput:
        if self.is_optional_out():
            yield f'''\
if (present({self._std_parameter.name})) then
    {self._std_parameter.name} = {self.internal_parameter().name}
end if
'''

    def decl_param(self) -> FortranArgument:
        param_info = copy.deepcopy(self._parameter_info)
        if self.is_buffer():
            param_info.descriptor.type.name = 'CHOICE_BUFFER_TYPE'
            param_info.descriptor.type.subtype = ''
            param_info.descriptor.dimension = None
        elif self._handle_status_ignore() or self._handle_statuses_ignore():
            if 'TARGET' not in param_info.descriptor.attributes:
                param_info.descriptor.attributes.append('TARGET')
        return param_info

    def is_buffer(self) -> bool:
        return self._std_parameter.kind.name == 'BUFFER'

    def is_optional_out(self) -> bool:
        return (self._parameter_info.descriptor.intent in ('OUT', 'INOUT')
                and 'OPTIONAL' in self._parameter_info.descriptor.attributes)

    def is_status(self) -> bool:
        return self._parameter_info.descriptor.type.name == 'TYPE' \
            and self._parameter_info.descriptor.type.subtype == 'MPI_STATUS' \
            and (self._parameter_info.descriptor.dimension is None
                 or self._parameter_info.descriptor.dimension == ['1'])

    def is_status_array(self) -> bool:
        return self._parameter_info.descriptor.type.name == 'TYPE' \
            and self._parameter_info.descriptor.type.subtype == 'MPI_STATUS' \
            and (self._parameter_info.descriptor.dimension is not None
                 and len(self._parameter_info.descriptor.dimension) == 1
                 and self._parameter_info.descriptor.dimension != ['1'])
