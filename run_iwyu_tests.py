#!/usr/bin/env python

##===--- run_iwyu_tests.py - include-what-you-use test framework driver ---===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""A test harness for IWYU testing."""

__author__ = 'dsturtevant@google.com (Dean Sturtevant)'

import glob
import os
import re
import sys
import unittest
import logging
logging.basicConfig(level=logging.INFO)
import posixpath
from fnmatch import fnmatch
import iwyu_test_util


def PosixPath(path):
    """Normalize Windows path separators to POSIX path separators."""
    return path.replace('\\', '/')


def Partition(l, delimiter):
  try:
    delim_index = l.index(delimiter)
  except ValueError:
    return l, []

  return l[:delim_index], l[delim_index+1:]


def GenerateTests(rootdir, pattern):
  def _GetTestBody(filename):
    def _test(self):
      logging.info('Testing iwyu on %s', filename)

      # Split full/path/to/foo.cc into full/path/to/foo and .cc.
      (all_but_extension, _) = os.path.splitext(filename)
      (dirname, basename) = os.path.split(all_but_extension)
      # Generate diagnostics on all foo-* files (well, not other
      # foo-*.cc files, which is not kosher but is legal), in addition
      # to foo.h (if present) and foo.cc.
      all_files = (glob.glob('%s-*' % all_but_extension) +
                   glob.glob('%s/*/%s-*' % (dirname, basename)) +
                   glob.glob('%s.h' % all_but_extension) +
                   glob.glob('%s/*/%s.h' % (dirname, basename)))
      files_to_check = [f for f in all_files if not fnmatch(f, pattern)]
      files_to_check.append(filename)

      # IWYU emits summaries with canonicalized filepaths, where all the
      # directory separators are set to '/'. In order for the testsuite to
      # correctly match up file summaries, we must canonicalize the filepaths
      # in the same way here.
      files_to_check = [PosixPath(f) for f in files_to_check]

      iwyu_test_util.TestIwyuOnRelativeFile(self, filename, files_to_check,
                                            verbose=True)
    return _test


  def _AddTestFunctions(cls):
    filenames = []
    for (dirpath, _, files) in os.walk(rootdir):
      dirpath = PosixPath(dirpath)  # Normalize path separators.
      filenames.extend(posixpath.join(dirpath, f) for f in files
                       if fnmatch(f, pattern))
    if not filenames:
      print('No tests found in %s!' % os.path.abspath(rootdir))
      return

    for filename in filenames:
      all_but_extension = os.path.splitext(filename)[0]
      basename = os.path.basename(all_but_extension)
      test_name = re.sub('[^0-9a-zA-Z_]', '_', basename)  # python-clean
      test_name = 'test_%s' % test_name

      while hasattr(cls, test_name):   # already have a class with that name
        test_name += '2'               # just append a suffix :-)

      logging.info('Registering %s.%s to test %s', cls.__name__, test_name,
                   filename)
      setattr(cls, test_name, _GetTestBody(filename))

    return cls

  return _AddTestFunctions


@GenerateTests(rootdir='tests/c', pattern='*.c')
class c(unittest.TestCase):
  pass


@GenerateTests(rootdir='tests/cxx', pattern='*.cc')
class cxx(unittest.TestCase):
  pass


if __name__ == '__main__':
  unittest_args, additional_args = Partition(sys.argv, '--')
  if additional_args:
    iwyu_test_util.SetIwyuPath(additional_args[0])

  unittest.main(argv=unittest_args)
