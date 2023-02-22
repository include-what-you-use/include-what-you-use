#!/usr/bin/env python3

##===--- iwyu-check-license-header.py - check license headers -------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

from __future__ import print_function
import sys
import os
import re
import argparse


# This is used in selected functions to calculate the maximum length of filename
# and filler dashes. Not otherwise useful.
EMBELLISHMENTS = '//===---  ---===//'

HDRFORMAT = """
{c}
{c}                     The LLVM Compiler Infrastructure
{c}
{c} This file is distributed under the University of Illinois Open Source
{c} License. See LICENSE.TXT for details.
{c}
{t}===----------------------------------------------------------------------==={t}
"""


def make_hdrformat(one, two):
    """ Materialize HDRFORMAT based on the one and two comment styles.

    The one and two args should be '#' and '##' or '//' and '//', respectively.
    Returns a list of lines.
    """
    r = HDRFORMAT.lstrip().format(c=one, t=two)
    return r.splitlines()


def truncated(filename):
    """ Truncate the filename with ellipsis if too long """
    maxlen = 80 - len(EMBELLISHMENTS)
    trunclen = maxlen - 3  # ...
    if len(filename) > maxlen:
        filename = filename[:trunclen] + '...'
    return filename


def make_license_header(filename, one, two):
    """ Build a valid license header from filename and comment styles.

    The one and two args should be '#' and '##' or '//' and '//', respectively.
    Returns a list of lines.
    """
    assert len(two) == 2
    filename = os.path.basename(filename)
    filename = truncated(filename)

    def dashes():
        c = 80
        c -= len(EMBELLISHMENTS)
        c -= len(filename)
        return '-' * c

    firstline = '%s===--- %s %s---===%s' % (two, filename, dashes(), two)
    return [firstline] + make_hdrformat(one, two)


def format_file_error(filename, *lines):
    """ Format an error message from filename and lines """
    lines = list(lines)
    lines[0] = '%s: %s' % (filename, lines[0])
    return os.linesep.join(lines)


def find_license_header(lines, two):
    """ Return an index where the license header begins """
    if not lines:
        return -1

    for i, line in enumerate(lines):
        # Allow leading blank lines and hash-bangs.
        if not line or line.startswith('#!'):
            continue

        # Besides those, the first line should be a license header.
        if line.startswith(two + '==='):
            break

        # If not, this fails the test entirely.
        return -1

    return i


class File(object):
    """ Base class for a source file with a license header

    Do not use directly, instead use File.parse to instantiate a more derived
    class.

    Derived classes must have three class variables:

    * one - One comment char ('#' for Python, '//' for C++)
    * two - Two comment chars ('##' for Python, '//' for C++)
    * pattern - a regex matching the first line in a license header
    """
    def __init__(self, filename):
        with open(filename, 'r') as fd:
            content = fd.read()

        self.lines = list(content.splitlines())
        self.hdrindex = find_license_header(self.lines, self.two)
        self.filename = filename
        self.errors = []

    @classmethod
    def parse(_, filename):
        """ Return an object derived from File to analyze license headers """
        _, ext = os.path.splitext(filename)
        if ext == '.py':
            klass = PythonFile
        elif ext in ('.h', '.c', '.cc'):
            klass = CxxFile
        else:
            return None

        return klass(filename)

    def has_license_header(self):
        """ Return True if a license header has been found """
        return self.hdrindex != -1

    def add_license_header(self):
        """ Add license header to a file that doesn't have one. """
        assert not self.has_license_header()

        # Find insertion point
        for p, line in enumerate(self.lines):
            # Skip past leading blank lines and hash-bangs.
            if line and not line.startswith('#!'):
                break

        # Split the lines around the insertion point
        if self.lines:
            before, after = self.lines[:p], self.lines[p:]
        else:
            before, after = [], []

        # Rebuild the contents with the license header in the middle
        lines = before
        if before and before[-1] != '':
            lines += ['']
        lines += make_license_header(self.filename, self.one, self.two)
        if after and after[0] != '':
            lines += ['']
        lines += after

        # Write back out
        content = '\n'.join(lines)
        with open(self.filename, 'wb') as fd:
            fd.write(content.encode('utf-8'))
            fd.write(b'\n')

    def check_license_header(self):
        """ Check that the header lines follow convention.

        Returns True if everything is OK, otherwise returns False and populates
        self.errors with all found errors.
        """
        if not self.has_license_header():
            self.file_error('No license header found')
            return False

        hdrlines = self.lines[self.hdrindex:self.hdrindex+8]

        # First line has the most structure
        line = hdrlines[0]
        if len(line) != 80:
            self.line_error(
                1, 'Bad header line length (expected: 80, was: %d)' % len(line),
                " Header line: '%s'" % line)

        m = self.pattern.match(line)
        if not m:
            self.line_error(1, 'Bad header line',
                            " Expected: '%s'" % self.pattern.pattern,
                            " Actual:   '%s'" % line)
        else:
            hfilename = truncated(m.group(1))
            xfilename = truncated(os.path.basename(self.filename))
            if hfilename != xfilename:
                self.line_error(1, 'Bad header filename',
                                " Expected:  '%s'" % xfilename,
                                " Actual:    '%s'" % hfilename)

        # The following seven lines always follow the layout of HDRFORMAT.
        hdrformat = make_hdrformat(self.one, self.two)
        hdrlines = hdrlines[1:]
        for lineno, (expected, actual) in enumerate(zip(hdrformat, hdrlines)):
            if expected != actual:
                self.line_error(lineno + 2, 'Bad header line',
                                " Expected: '%s'" % expected,
                                " Actual:   '%s'" % actual)

        return not self.errors

    def file_error(self, *lines):
        """ Log an error for the file """
        self.errors.append(format_file_error(self.filename, *lines))

    def line_error(self, lineno, *lines):
        """ Log an error for a specific line in the file """
        lines = list(lines)
        lines[0] = '%s:%d: %s' % (self.filename, lineno, lines[0])
        self.errors.append(os.linesep.join(lines))


class PythonFile(File):
    """ Python file with license header """
    one = '#'
    two = '##'
    pattern = re.compile(
        r'##===--- ([a-z0-9_.-]+) -[A-Za-z0-9_.,/# -{}]+===##')

    def __init__(self, filename):
        super(PythonFile, self).__init__(filename)


class CxxFile(File):
    """ C++ file with license header """
    one = '//'
    two = '//'
    pattern = re.compile(
        r'//===--- ([a-z0-9_.-]+) -[A-Za-z0-9_.,/# -{}]+ [-* C+]+===//')

    def __init__(self, filename):
        super(CxxFile, self).__init__(filename)


def main(filenames, add_if_missing):
    """ Entry point.

    Checks license header of all filenames provided.
    Returns zero if all license headers are OK, non-zero otherwise.
    """
    errors = []
    for filename in filenames:
        if os.path.isdir(filename):
            continue

        checker = File.parse(filename)
        if not checker:
            # TODO: Consider printing a warning here in verbose mode.
            continue

        if not checker.check_license_header():
            errors.extend(checker.errors)

        if add_if_missing and not checker.has_license_header():
            checker.add_license_header()

    for err in errors:
        print(err)

    return len(errors)


if __name__ == '__main__':
    parser = argparse.ArgumentParser('IWYU license header checker')
    parser.add_argument('filename', nargs='+')
    parser.add_argument('--add', action='store_true')
    args = parser.parse_args()
    sys.exit(main(args.filename, args.add))
