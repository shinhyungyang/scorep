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

from . import template_interface
from . import logs
from . import errors

from wrapgen.generator.wrapper_hooks import GeneratorOutput, GeneratorFunction
from wrapgen.generator.wrapper import Wrapper
from .generator.task import ProcedureTask
from .generator.scope import Scope
from wrapgen.generator.scorep_procedure import ScorepProcedure
from .generator.scorep_parameter import ScorepParameter
from wrapgen.generator.c.wrapper_tasks.ctask import CTask
from wrapgen.generator.f08.wrapper_tasks.f08task import F08Task

from . import data
from . import generator


def init(enable_debug_output=False):
    data.init()
    generator.init(enable_debug_output)
