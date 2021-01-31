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


class OneIwyuTest(unittest.TestCase):
  """Superclass for tests.  A subclass per test-file is created at runtime."""

  def RunOneTest(self, filename):
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
    files_to_check = [f for f in all_files if not fnmatch(f, self.pattern)]
    files_to_check.append(filename)

    # IWYU emits summaries with canonicalized filepaths, where all the
    # directory separators are set to '/'. In order for the testsuite to
    # correctly match up file summaries, we must canonicalize the filepaths
    # in the same way here.
    files_to_check = [PosixPath(f) for f in files_to_check]

    iwyu_test_util.TestIwyuOnRelativeFile(self, filename, files_to_check,
                                          verbose=True)


def RegisterFilesForTesting(rootdir, pattern):
  """Create a test-class for every file in rootdir matching pattern."""
  filenames = []
  for (dirpath, dirs, files) in os.walk(rootdir):
    dirpath = PosixPath(dirpath)  # Normalize path separators.
    filenames.extend(posixpath.join(dirpath, f) for f in files
                     if fnmatch(f, pattern))
  if not filenames:
    print('No tests found in %s!' % os.path.abspath(rootdir))
    return

  module = sys.modules[__name__]

  for filename in filenames:
    all_but_extension = os.path.splitext(filename)[0]
    basename = os.path.basename(all_but_extension)
    class_name = re.sub('[^0-9a-zA-Z_]', '_', basename)  # python-clean
    if class_name[0].isdigit():            # classes can't start with a number
      class_name = '_' + class_name
    while class_name in module.__dict__:   # already have a class with that name
      class_name += '2'                    # just append a suffix :-)

    logging.info('Registering %s to test %s', class_name, filename)
    test_class = type(class_name,          # class name
                      (OneIwyuTest,),      # superclass
                      # and attrs. f=filename is required for proper scoping
                      {'runTest': lambda self, f=filename: self.RunOneTest(f),
                       'pattern': pattern})
    setattr(module, test_class.__name__, test_class)


if __name__ == '__main__':
  unittest_args, additional_args = Partition(sys.argv, '--')
  if additional_args:
    iwyu_test_util.SetIwyuPath(additional_args[0])

  RegisterFilesForTesting('tests/cxx', '*.cc')
  RegisterFilesForTesting('tests/c', '*.c')
  unittest.main(argv=unittest_args)
