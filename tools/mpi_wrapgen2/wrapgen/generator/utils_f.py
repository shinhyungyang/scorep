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
from .utils import generate_lines
from .wrapper_hooks import GeneratorOutput


def generate_multiline_comment(*msg: str) -> GeneratorOutput:
    for line in generate_lines(*msg):
        yield from ('!', line)
    yield '\n'
