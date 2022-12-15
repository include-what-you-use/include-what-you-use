#!/usr/bin/env python3

##===--- scrub-logs.py - generate README from Wiki sources ----------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Scrub irrelevant details from IWYU/Clang logs.

When Clang changes upstream, we usually look for differences in the AST to
explain the new behavior. This script makes that easier by scrubbing pointer
values and path prefixes from ast-dump output, so they can be diffed directly.
"""

import re
import sys
import fileinput


def strip_path_prefix(line):
    line = re.sub(r'<.*(llvm[\\/]tools[\\/].*):', r'<\1:', line)
    return line


def strip_addrs(line):
    line = re.sub(r'\b(0x)?[0-9A-Fa-f]{6,16}', '', line)
    return line


def main():
    for line in fileinput.input():
        line = line.strip()
        line = strip_addrs(line)
        line = strip_path_prefix(line)
        print(line)

    return 0


if __name__ == '__main__':
    sys.exit(main())
