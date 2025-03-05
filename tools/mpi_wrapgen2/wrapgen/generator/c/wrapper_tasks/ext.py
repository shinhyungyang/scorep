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

from wrapgen.generator.c.wrapper_tasks import CTask


class TaskAbort(CTask):
    def generate_initialization(self):
        yield 'UTILS_WARNING( "Explicit MPI_Abort call abandoning the SCOREP measurement." );\n'
        yield 'SCOREP_SetAbortFlag();\n'
