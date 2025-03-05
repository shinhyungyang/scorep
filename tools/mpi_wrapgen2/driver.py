#!/usr/bin/env python3

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
import json
import logging
import argparse
from pathlib import Path

from jinja2 import Environment, FileSystemLoader, select_autoescape

import wrapgen
from wrapgen import errors
from wrapgen.generator import GENERATED_WRAPPERS, WrapperStatus, Layer
from wrapgen.data import ProcedureSets, get_predefined_procedure_set

_logger = logging.getLogger(f'{wrapgen.logs.get_root_logger_name()}.driver')


def main():
    template_root = Path('templates')
    output_root = Path('output')

    parser = argparse.ArgumentParser(
        description=f"Driver for generating wrapper code from the template files in '{template_root}'. "
                    f"Generated files are put into '{output_root}'."
    )
    parser.add_argument('--match', nargs='+', default=('adapter', 'm4'))
    parser.add_argument('--enable-debug-output', action='store_true',
                        help='Generate wrappers with debug comments.')
    parser.add_argument('--permissive', action='store_true',
                        help='Warn and continue on some recoverable errors.')
    args = parser.parse_args()

    wrapgen.logs.set_default_logging_behavior(logfile='wrapgen-driver')
    wrapgen.init(enable_debug_output=args.enable_debug_output)

    if args.permissive:
        wrapgen.errors.be_permissive()
    else:
        wrapgen.errors.be_strict()

    env = Environment(
        loader=FileSystemLoader(template_root),
        autoescape=select_autoescape([]),
        trim_blocks=False,
        lstrip_blocks=False,
        keep_trailing_newline=True
    )

    output_root.mkdir(parents=False, exist_ok=True)

    def _path_in_templates(path: Path):
        return path.relative_to(template_root)

    def _output_path(path: Path):
        return output_root.joinpath(_path_in_templates(path))

    def _generate_from_template(path: Path):
        if not path.is_file() or path.suffix != '.tmpl':
            errors.error_if_strict(f"'{path}' is not a valid template file.", _logger)
        output_file_path = _output_path(path).with_suffix('')
        _logger.info(f"Generating '{output_file_path}' from '{path}'")
        template = env.get_template(_path_in_templates(path).as_posix())
        with open(output_file_path, 'w') as out:
            out.write(template.render(wrapgen=wrapgen.template_interface))

    def _mkdir_in_output(path: Path):
        output_path = _output_path(path)
        output_path.mkdir(parents=False, exist_ok=True)

    def _related(path1: Path, path2: Path):
        return path1.is_relative_to(path2) or path2.is_relative_to(path1)

    def _recurse_template_dir(cur: Path):
        item_in_templates = _path_in_templates(cur)
        if item_in_templates.match('_*'):
            _logger.debug(f"Excluded path '{cur}': Path starts with '_'")
            return
        include = any((_related(cur, template_root.joinpath(m)) for m in args.match))
        if not include:
            _logger.debug(f"Excluded path '{cur}': Path does not match the filter")
            return
        _logger.info(f"Looking in '{cur}'")
        _mkdir_in_output(cur)
        for item in cur.iterdir():
            if item.is_dir():
                _recurse_template_dir(item)
            elif item.is_file():
                _generate_from_template(item)

    _recurse_template_dir(template_root)
    write_report()


def write_report():
    _logger.info('Writing report')

    layer_to_procedure_set = {
        Layer.C: ProcedureSets.ALL_ISO_C,
        Layer.F: ProcedureSets.ALL_F90_C,
        Layer.F08: ProcedureSets.ALL_F08
    }

    report = {'excluded': sorted(get_predefined_procedure_set(ProcedureSets.EXCLUDED))}
    for layer, set_name in layer_to_procedure_set.items():
        expressible = set(get_predefined_procedure_set(set_name))
        encountered = set(GENERATED_WRAPPERS[layer].keys())
        report[layer.name] = {
            'not seen': sorted(list(expressible - encountered)),
            'todo': sorted([name for name, status in GENERATED_WRAPPERS[layer].items()
                            if status is WrapperStatus.TODO]),
            'done': sorted([name for name, status in GENERATED_WRAPPERS[layer].items()
                            if status is WrapperStatus.DONE])
        }

    counts = {}
    for layer in map(lambda k: k.name, layer_to_procedure_set.keys()):
        counts[layer] = {'all': 0}
        for status, proc_names in report[layer].items():
            counts[layer][status] = len(proc_names)
            counts[layer]['all'] += counts[layer][status]

    num_excluded = len(report['excluded'])
    num_all_procedures_in_std = len(get_predefined_procedure_set(ProcedureSets.ALL)) + num_excluded
    _logger.info(f'Excluded {num_excluded} of the {num_all_procedures_in_std} procedures in the MPI Standard.')

    def make_row(*args):
        return '   '.join((f'{str(a):>8s}' for a in args))

    progress_table = [
        'Progress statistics by layer:',
        make_row('layer', 'not seen', 'todo', 'done', 'all'),
        '-' * 52]
    for layer_name, layer_counts in map(lambda k: (k.name, counts[k.name]), layer_to_procedure_set.keys()):
        progress_table.append(make_row(layer_name, layer_counts['not seen'], layer_counts['todo'], layer_counts['done'],
                                       layer_counts['all']))
    _logger.info("\n".join(progress_table))

    with open('report', 'w') as report_file:
        json.dump(report, fp=report_file, indent=4)


if __name__ == '__main__':
    main()
