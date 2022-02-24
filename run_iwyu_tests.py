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

import argparse
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


def TestIwyuOnRelevantFiles(filename):
  logging.info('Testing iwyu on %s', filename)
  # Split full/path/to/foo.cc into full/path/to/foo and .cc.
  (all_but_extension, extension) = os.path.splitext(filename)
  (dirname, basename) = os.path.split(all_but_extension)
  # Generate diagnostics on all foo-* files (well, not other
  # foo-*.cc files, which is not kosher but is legal), in addition
  # to foo.h (if present) and foo.cc.
  all_files = (glob.glob('%s-*' % all_but_extension) +
               glob.glob('%s/*/%s-*' % (dirname, basename)) +
               glob.glob('%s.h' % all_but_extension) +
               glob.glob('%s/*/%s.h' % (dirname, basename)))
  
  files_to_check = [f for f in all_files if not f.endswith(extension)]
  files_to_check.append(filename)

  # IWYU emits summaries with canonicalized filepaths, where all the
  # directory separators are set to '/'. In order for the testsuite to
  # correctly match up file summaries, we must canonicalize the filepaths
  # in the same way here.
  files_to_check = [PosixPath(f) for f in files_to_check]
  iwyu_test_util.TestIwyuOnRelativeFile(filename, files_to_check, verbose=True)


def GenerateTests(rootdir, pattern):
  def _AddTestFunctions(cls):
    filenames = []
    test_files = {}
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

      setattr(cls, test_name, lambda x, f=filename: TestIwyuOnRelevantFiles(f))
      test_files[test_name] = filename

    setattr(cls, 'test_files', test_files)

    return cls

  return _AddTestFunctions


def EnumerateLoadedTests():
  for suite in unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__]):
    for test in suite:
      yield (test.__class__, test._testMethodName)


def PrintLoadedTests():
  for (cls, test) in EnumerateLoadedTests():
    print('%s.%s' % (cls.__name__, test))


def PrintLoadedTestsAndFiles():
  for (cls, test) in EnumerateLoadedTests():
    print('%s.%s:%s' % (cls.__name__, test, cls.test_files[test]))


if __name__ == '__main__':
  unittest_args, additional_args = Partition(sys.argv, '--')
  if additional_args:
    iwyu_test_util.SetIwyuPath(additional_args[0])

  parser = argparse.ArgumentParser(add_help=False, usage=argparse.SUPPRESS)
  group = parser.add_mutually_exclusive_group()
  group.add_argument('--list', dest='list_tests', action='store_true')
  group.add_argument('--list-test-files', action='store_true')
  group.add_argument('--run-test-file')
  (runner_args, _) = parser.parse_known_args(unittest_args)

  if runner_args.run_test_file:
    exit(TestIwyuOnRelevantFiles(runner_args.run_test_file))

  @GenerateTests(rootdir='tests/c', pattern='*.c')
  class c(unittest.TestCase): pass

  @GenerateTests(rootdir='tests/cxx', pattern='*.cc')
  class cxx(unittest.TestCase): pass

  @GenerateTests(rootdir='tests/driver', pattern='*.c')
  class driver(unittest.TestCase): pass

  if runner_args.list_tests:
    exit(PrintLoadedTests())
  elif runner_args.list_test_files:
    exit(PrintLoadedTestsAndFiles())

  unittest.main(argv=unittest_args)
