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
import unittest
import logging
logging.basicConfig(level=logging.INFO)
class Flags(object):
   def __init__(self): self.test_files = []
FLAGS = Flags()
import iwyu_test_util


TEST_ROOTDIR = 'tests'


class IwyuTests(unittest.TestCase):
  def setUp(self):
    # Iwyu flags for specific tests.
    # Map from filename to flag list. If any test requires special
    # iwyu flags to run properly, add an entry to the map with
    # key=filename (starting with TEST_ROOTDIR), value=list of flags.
    self._iwyu_flags_map = {}

  def testIwyuWorks(self):
    files_to_test = [os.path.join(TEST_ROOTDIR, filename)
                     for filename in FLAGS.test_files]
    if not files_to_test:
      files_to_test = glob.glob(os.path.join(TEST_ROOTDIR, '*.cc'))
    failed_tests = []
    for filename in files_to_test:
      logging.info('Testing iwyu on %s', filename)

      # Split full/path/to/foo.cc into full/path/to/foo and .cc.
      (basename, extension) = os.path.splitext(filename)
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
        logging.info('Using iwyu flags %s', str(iwyu_flags))

      try:
        iwyu_test_util.TestIwyuOnRelativeFile(self, filename, files_to_check)
      except AssertionError, why:
        logging.error('Test failed for %s\n---%s---\n' % (filename, why))
        failed_tests.append(filename)

    self.assert_(not failed_tests, 'Failed tests: %s' % ' '.join(failed_tests))

if __name__ == '__main__':

  unittest.main()
