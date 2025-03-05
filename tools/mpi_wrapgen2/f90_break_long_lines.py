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
import argparse
import re

_FORTRAN_LINE_LENGTH_LIMIT = 132


def yield_continued_lines(file_line):
    """
    Break an actual line (a line in a file ended with '\n', not a Fortran logical line) into chunks of
    the maximal line length

    Keep the leading whitespace intact for continued lines
    Does not work for lines that contain a mixture of comment and code
    :param file_line:
    :return:
    """
    # Do not split preprocessor directives
    m = re.match(r'^#', file_line)
    if m:
        yield file_line
        return

    stripped_line = file_line.rstrip('\n')
    m = re.match(r'(\s*)(.*)', stripped_line)
    leading_whitespace = m.group(1)
    leading_whitespace_length = len(leading_whitespace)
    rest = m.group(2)
    is_comment = (re.match(r'!', rest) is not None)

    line_length = _FORTRAN_LINE_LENGTH_LIMIT - leading_whitespace_length
    while len(rest) > line_length:
        line_part = rest[:line_length - 1]
        rest = rest[line_length - 1:]
        if is_comment:
            rest = '!' + rest
        else:
            line_part += '&'
            rest = '&' + rest
        line_part = leading_whitespace + line_part + '\n'
        yield line_part
    yield leading_whitespace + rest + '\n'


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('input')
    parser.add_argument('output')
    args = parser.parse_args()

    with open(args.input, 'r') as infile:
        with open(args.output, 'w') as outfile:
            for line in infile:
                for part in yield_continued_lines(line):
                    print(part, end='', file=outfile)
