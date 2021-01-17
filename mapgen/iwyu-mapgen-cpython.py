#!/usr/bin/env python

##===--- iwyu-mapgen-cpython.py -------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

""" Generate mappings for Python C API headers.

The Python include root in e.g. /usr/include/python3.8 contains a single public
header: Python.h.

Simply collect all included header names and map them all to Python.h.
"""
import os
import re
import sys
import json
import argparse
import fnmatch


INCLUDE_RE = re.compile(r'#\s*include\s+"([^"]+)"')


def parse_include_names(headerpath):
    """ Parse the header file at headerpath and return all include names. """
    with open(headerpath, 'r') as fobj:
        for line in fobj.readlines():
            m = INCLUDE_RE.search(line)
            if m:
                yield m.group(1)


def iterfiles(dirpath, pattern):
    """ Recursively find all files matching pattern. """
    for root, _, files in os.walk(dirpath):
        for fname in files:
            if fnmatch.fnmatch(fname, pattern):
                yield os.path.join(root, fname)


def generate_imp_lines(include_names):
    """ Generate a sequence of json-formatted strings in .imp format.

    This should ideally return a jsonable structure instead, and use json.dump
    to write it to the output file directly. But there doesn't seem to be a
    simple way to convince Python's json library to generate a "packed"
    formatting, it always prefers to wrap dicts onto multiple lines.

    Cheat, and use json.dumps for escaping each line.
    """
    def jsonline(mapping, indent):
        return (indent * ' ') + json.dumps(mapping)

    for name in sorted(include_names):
        # Regex-escape period and build a regex matching both "" and <>.
        map_from = r'@["<]%s[">]' % name.replace('.', '\\.')
        mapping = {'include': [map_from, 'private', '<Python.h>', 'public']}
        yield jsonline(mapping, indent=2)


def main(pythonroot):
    """ Entry point. """

    # Collect all include names in the root. These are the private includes.
    included_names = []
    for fname in iterfiles(pythonroot, '*.h'):
        included_names.extend(parse_include_names(fname))

    # Discard duplicates and remove Python.h itself.
    included_names = set(included_names)
    included_names.remove('Python.h')

    # Print mappings from name -> Python.h.
    print('[')
    print(',\n'.join(generate_imp_lines(sorted(included_names))))
    print(']')

    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate IWYU mappings for the Python C API.')
    parser.add_argument('pythonroot',
                        help='Python include root (e.g. /usr/include/python3.8')
    args = parser.parse_args()
    sys.exit(main(args.pythonroot))
