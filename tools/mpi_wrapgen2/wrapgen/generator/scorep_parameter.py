import abc
from typing import Optional

from wrapgen.generator.wrapper_hooks import GeneratorOutput
from wrapgen.data import mpistandard as std
from wrapgen.language import ParameterBase


class ScorepParameter(abc.ABC):
    def __init__(self, **kwargs):
        self._extra_info = kwargs

    @abc.abstractmethod
    def _get_std_parameter(self) -> std.Parameter:
        pass

    @property
    @abc.abstractmethod
    def parameter(self) -> ParameterBase:
        pass

    @property
    def name(self) -> str:
        return self.parameter.name

    @abc.abstractmethod
    def internal_parameter(self) -> ParameterBase:
        pass

    def has_internal_replacement(self) -> bool:
        return False

    def generate_init(self) -> GeneratorOutput:
        yield from ()

    def generate_cleanup(self) -> GeneratorOutput:
        yield from ()

    def _handle_status_ignore(self) -> bool:
        return self._extra_info.get('handle-status-ignore', False)

    def _handle_statuses_ignore(self) -> bool:
        return self._extra_info.get('handle-statuses-ignore', False)

    def _status_array_length(self) -> Optional[str]:
        return self._extra_info.get('statuses-length', None)
