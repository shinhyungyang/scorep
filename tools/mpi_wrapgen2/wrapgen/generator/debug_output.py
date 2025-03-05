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
_debug_output_enabled = False


def enable_debug_output():
    global _debug_output_enabled
    _debug_output_enabled = True


def disable_debug_output():
    global _debug_output_enabled
    _debug_output_enabled = False


def is_debug_enabled():
    return _debug_output_enabled
