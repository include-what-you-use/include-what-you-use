#!/usr/bin/python

##===--- run_iwyu_tests.py - include-what-you-use test framework driver ----===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===-----------------------------------------------------------------------===##

"""A test harness for IWYU testing."""



import glob
import os
import re
import sys
import unittest
import logging
logging.basicConfig(level=logging.INFO)
class Flags(object):
   def __init__(self): self.test_files = []
FLAGS = Flags()
import iwyu_test_util


TEST_ROOTDIR = 'tests'


class OneIwyuTest(unittest.TestCase):
  """Superclass for tests.  A subclass per test-file is created at runtime."""

  def setUp(self):
    # Iwyu flags for specific tests.
    # Map from filename to flag list. If any test requires special
    # iwyu flags to run properly, add an entry to the map with
    # key=filename (starting with TEST_ROOTDIR), value=list of flags.
    self._iwyu_flags_map = {}

  def RunOneTest(self, filename):
    logging.info('Testing iwyu on %s', filename)
    # Split full/path/to/foo.cc into full/path/to/foo and .cc.
    (basename, _) = os.path.splitext(filename)
    # Generate diagnostics on all foo-*.h files in addition to
    # foo.h (if present) and foo.h.
    files_to_check = glob.glob(basename + '-*.h')
    h_file = basename + '.h'
    if os.path.exists(h_file):
      files_to_check.append(h_file)
    files_to_check.append(filename)

    iwyu_flags = None
    if filename in self._iwyu_flags_map:
      iwyu_flags = self._iwyu_flags_map[filename]
      logging.info('%s: Using iwyu flags %s', filename, str(iwyu_flags))

    iwyu_test_util.TestIwyuOnRelativeFile(self, filename, files_to_check)


def setUp():
  """Create a test-class for every .cc file in TEST_ROOTDIR."""
  module = sys.modules[__name__]
  for filename in glob.glob(os.path.join(TEST_ROOTDIR, '*.cc')):
    basename = os.path.basename(filename[:-len('.cc')])
    class_name = re.sub('[^0-9a-zA-Z_]', '_', basename)  # python-clean
    if class_name[0].isdigit():            # classes can't start with a number
      class_name = '_' + class_name
    while class_name in module.__dict__:   # already have a class with that name
      class_name += '2'                    # just append a suffix :-)

    logging.info('Registering %s to test %s', class_name, filename)
    test_class = type(class_name,          # <-- class name
                      (OneIwyuTest,),      # <-- superclass, v-- methods
                      {'runTest': lambda self, f=filename: self.RunOneTest(f)})
    setattr(module, test_class.__name__, test_class)


if __name__ == '__main__':

  unittest.main()
