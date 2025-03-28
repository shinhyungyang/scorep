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
__all__ = ['TaskAbort']

from wrapgen.generator.f08.wrapper_tasks import F08Task
from wrapgen.generator.f08 import scorep_f08_macros


class TaskAbort(F08Task):
    def generate_initialization(self):
        yield f'''\
{scorep_f08_macros.UTILS_WARNING("Explicit MPI_Abort call abandoning the Score-P measurement.")}
call SCOREP_SetAbortFlag()
'''
