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
scorep_pure_groups = (
    'cg',
    'coll',
    'env',
    'err',
    'ext',
    'io',
    'misc',
    'p2p',
    'rma',
    'spawn',
    'topo',
    'type',
    'perf',
    'xnonblock',
    'xreqtest'
)

scorep_derived_groups = (
    ('cg', 'err'),
    ('cg', 'ext'),
    ('cg', 'misc'),
    ('io', 'err'),
    ('io', 'misc'),
    ('rma', 'err'),
    ('rma', 'ext'),
    ('rma', 'misc'),
    ('type', 'ext'),
    ('type', 'misc')
    # treat 'request' specially
)

scorep_all_groups = tuple((g for g in scorep_pure_groups if g != 'xnonblock'))

scorep_default_enabled_groups = (
    'cg',
    'coll',
    'env',
    'io',
    'p2p',
    'rma',
    'topo',
)
