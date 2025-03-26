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
from typing import Optional
import logging


def get_root_logger_name() -> str:
    return 'wrapgen'


def get_root_logger() -> logging.Logger:
    return logging.getLogger(get_root_logger_name())


def set_default_logging_behavior(logfile: Optional[str] = None,
                                 root_name: str = get_root_logger_name()) -> None:
    logger = logging.getLogger(root_name)

    logger.setLevel(logging.DEBUG)

    simple_format = logging.Formatter('%(levelname)s: %(message)s')
    detailed_format = logging.Formatter('%(asctime)s: %(levelname)s in %(name)s: %(message)s')

    if logfile is not None:
        fh = logging.FileHandler('{0}.log'.format(logfile), mode='w')
        fh.setLevel(logging.DEBUG)
        fh.setFormatter(detailed_format)
        logger.addHandler(fh)

    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    ch.setFormatter(simple_format)
    logger.addHandler(ch)
