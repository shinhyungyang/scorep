import copy

from wrapgen.generator.wrapper_hooks import GeneratorOutput
from wrapgen.generator.scorep_parameter import ScorepParameter
from wrapgen.data import mpistandard as std
from wrapgen.language.iso_c import CParameter
from wrapgen.language.pympistandard_to_c import make_c_parameter


class ScorepCParameter(ScorepParameter):
    def __init__(self, std_parameter: std.ISOCParameter, **kwargs):
        super(ScorepCParameter, self).__init__(**kwargs)
        self._std_parameter: std.ISOCParameter = std_parameter
        self._parameter_info: CParameter = make_c_parameter(std_parameter)

    @property
    def _get_std_parameter(self) -> std.ISOCParameter:
        return self._std_parameter

    @property
    def parameter(self) -> CParameter:
        return self._parameter_info

    def internal_parameter(self) -> CParameter:
        param_info = copy.deepcopy(self._parameter_info)
        if self.has_internal_replacement():
            param_info.name = f'internal_{param_info.name}'
        return param_info

    def has_internal_replacement(self) -> bool:
        return False

    def generate_init(self) -> GeneratorOutput:
        if self._handle_status_ignore():
            assert self.is_status_pointer()
            yield f'''\
if ( {self._parameter_info.name} == MPI_STATUS_IGNORE )
{{
    {self._parameter_info.name} = scorep_mpi_get_status_array(1);
}}
'''
        elif self._handle_statuses_ignore():
            assert self.is_status_array()
            assert self._status_array_length() is not None
            yield f'''\
if ( {self._parameter_info.name} == MPI_STATUSES_IGNORE )
{{
    {self._parameter_info.name} = scorep_mpi_get_status_array( {self._status_array_length()} );
}}
'''

    def generate_cleanup(self) -> GeneratorOutput:
        yield from ()

    def is_status_pointer(self) -> bool:
        return self._parameter_info.base_type == 'MPI_Status' \
            and self._parameter_info.pointer_level == 1 \
            and len(self._parameter_info.arrays) == 0

    def is_status_array(self) -> bool:
        return self._parameter_info.base_type == 'MPI_Status' \
            and self._parameter_info.pointer_level == 0 \
            and len(self._parameter_info.arrays) == 1
