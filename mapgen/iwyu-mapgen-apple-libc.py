#!/usr/bin/env python3

##===--- iwyu-mapgen-apple-libc.py ----------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

""" Generate mappings for Apple Libc.

Map headers that either start with an underscore or are architecture-specific.
"""
import argparse
import json
import sys

from operator import itemgetter
from pathlib import Path


PATTERN_MAPPINGS = {}


def main(dir, verbose):
    """ Entry point. """

    root = Path(dir)

    paths = set(root.glob('**/*.h'))

    mappings = []

    for p in paths:
        initial = p

        # eg. xlocale/_stdio.h -> _stdio.h
        if p.parent.name == 'xlocale':
            public_path = p.parent.with_name(p.name)
            if public_path in paths:
                p = public_path

        # eg. _stdio.h -> stdio.h
        while p.name.startswith('_'):
            public_path = p.with_name(p.name[1:])
            if public_path in paths:
                p = public_path
            else:
                break

        # arm -> machine
        if p.parent.name in {'arm', 'arm64', 'i386'}:
            public_path = p.parent.with_name('machine') / p.name
            if public_path in paths:
                p = public_path

        # eg. machine/limits.h -> limits.h
        # do not map machine/profile.h to profile.h which is a kerberos header
        if p.parent.name == 'machine' and p.name != 'profile.h':
            public_path = p.parent.with_name(p.name)
            if public_path in paths:
                p = public_path

        # map other headers
        for glob, repl in PATTERN_MAPPINGS.items():
            if p.relative_to(root).full_match(glob):
                public_path = root / repl
                if public_path in paths:
                    p = public_path
                break

        # we don't know this one
        if p.name.startswith('_'):
            if verbose:
                print(f'could not map {p.relative_to(root)}', file=sys.stderr)
            continue

        if p != initial:
            mappings.append([
                f'<{initial.relative_to(root)}>',
                'private',
                f'<{p.relative_to(root)}>',
                'public'
            ])

    mappings.sort(key=itemgetter(0))

    print(f'# Generated using {' '.join(sys.argv)}')
    print('[')
    for mapping in mappings:
        print(end='    ')
        json.dump(dict(include=mapping), sys.stdout)
        print(',')
    print(']')

    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate IWYU mappings for Apple Libc.'
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="verbose output"
    )
    parser.add_argument(
        'includedir',
        help='Include directory (e.g. `xcrun --show-sdk-path`/usr/include)'
    )
    args = parser.parse_args()
    sys.exit(main(args.includedir, args.verbose))
