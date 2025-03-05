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
from .wrapper_hooks import GeneratorOutput


def generate_lines(*strings: str) -> GeneratorOutput:
    line_buf = []
    for s in strings:
        lines = s.split('\n')
        for line in lines[:-1]:
            line_buf.append(line)
            line_buf.append('\n')
            yield ''.join(line_buf)
            line_buf = []
        line_buf.append(lines[-1])
    yield ''.join(line_buf)
