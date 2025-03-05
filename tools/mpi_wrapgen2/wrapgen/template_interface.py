#
# This file is part of the Score-P software (http://www.score-p.org)
#
# Copyright (c) 2025,
# Forschungszentrum Juelich GmbH, Germany
#
# This software may be modified and distributed under the terms of
# a BSD-style license. See the COPYING file in the package base
# directory for details.
#
"""
This module exposes a very limited interface from :mod:`wrapgen` to the
template engine.

The template files only provide the file-level structure: file headers,
auxiliary declarations, comments, etc., and where to generate each procedure.
There is *absolutely no information* how to generate a procedure in the templates:
The generator needs only the *name* and *binding language* to generate each
procedure.  Everything else comes from :mod:`wrapgen.data.mpistandard`
and :mod:`wrapgen.data.scorep`.

The only way to determine what and how things are generated is via the
combination of

* the list of procedure names
* a generator callback

In the template engine, the only thing known about a procedure is its name
(at least concerning the public interface, nothing is really inaccessible in Python).

There should be no computation in the templates, for example

* filtering lists of procedures based on their name or properties
* toggle generating specific code based on procedure properties

"""
__all__ = ['ProcedureSets',
           'get_num_procedures_in_set',
           'generate_for_procedures',
           'callbacks',
           'generate_c_group_enum',
           'generate_c_enable_derived_groups',
           'generate_f08_group_params',
           'generate_ac_c_symbol_checks',
           ]

from typing import Union, Iterable, Callable, Iterator, cast

from wrapgen.data import ProcedureSets, get_num_procedures_in_set, ProcedureData
from wrapgen import callbacks

import jinja2 as _jinja2
from wrapgen.generator import (string_from_generator_output as _string_from_generator_output,
                               generate_for_procedures as _generate_procedures)
from wrapgen.generator.c import (generate_c_group_enum as _generate_c_group_enum,
                                 generate_c_enable_derived_groups as _generate_c_enable_derived_groups)
from wrapgen.generator.f08 import generate_f08_group_params as _generate_f08_group_params
from wrapgen.generator.autoconf import generate_ac_c_symbol_checks as _generate_ac_c_symbol_checks


def generate_for_procedures(procedures: Union[ProcedureSets, Iterable[str]],
                            callback: Callable[[ProcedureData], Iterator[str]],
                            sort: bool = False) -> str:
    # Force raising an undefined exception early
    if _jinja2.is_undefined(callback):
        # noinspection PyProtectedMember
        cast(_jinja2.Undefined, callback)._fail_with_undefined_error()
    return _string_from_generator_output(_generate_procedures(procedures, callback, sort))


def generate_c_group_enum() -> str:
    return _string_from_generator_output(_generate_c_group_enum())


def generate_c_enable_derived_groups() -> str:
    return _string_from_generator_output(_generate_c_enable_derived_groups())


def generate_f08_group_params() -> str:
    return _string_from_generator_output(_generate_f08_group_params())


def generate_ac_c_symbol_checks() -> str:
    return _string_from_generator_output(_generate_ac_c_symbol_checks())
