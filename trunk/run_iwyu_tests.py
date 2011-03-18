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


def CheckAlsoExtension(extension):
  """Return a suitable iwyu flag for checking files with the given extension."""
  return '--check_also="%s"' % os.path.join(TEST_ROOTDIR, '*' + extension)


class OneIwyuTest(unittest.TestCase):
  """Superclass for tests.  A subclass per test-file is created at runtime."""

  def setUp(self):
    # Iwyu flags for specific tests.
    # Map from filename to flag list. If any test requires special
    # iwyu flags to run properly, add an entry to the map with
    # key=cc-filename (relative to TEST_ROOTDIR), value=list of flags.
    flags_map = {
      'check_also.cc': [CheckAlsoExtension('-d1.h')],
      'implicit_ctor.cc': [CheckAlsoExtension('-d1.h')],
      'keep_mapping.cc': [CheckAlsoExtension('-public.h')],
      'overloaded_class.cc': [CheckAlsoExtension('-i1.h')],
    }
    # Internally, we like it when the paths start with TEST_ROOTDIR.
    self._iwyu_flags_map = dict((os.path.join(TEST_ROOTDIR, k), v)
                                for (k,v) in flags_map.items())

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

    iwyu_flags = self._iwyu_flags_map.get(filename, None)
    if iwyu_flags:
      logging.info('%s: Using iwyu flags %s', filename, str(iwyu_flags))

    iwyu_test_util.TestIwyuOnRelativeFile(self, filename, files_to_check,
                                          iwyu_flags, verbose=True)


def RegisterFilesForTesting():
  """Create a test-class for every .cc file in TEST_ROOTDIR."""
  module = sys.modules[__name__]
  filenames = glob.glob(os.path.join(TEST_ROOTDIR, '*.cc'))
  if not filenames:
    sys.exit('No tests found in %s!' % os.path.abspath(TEST_ROOTDIR))
  for filename in filenames:
    basename = os.path.basename(filename[:-len('.cc')])
    class_name = re.sub('[^0-9a-zA-Z_]', '_', basename)  # python-clean
    if class_name[0].isdigit():            # classes can't start with a number
      class_name = '_' + class_name
    while class_name in module.__dict__:   # already have a class with that name
      class_name += '2'                    # just append a suffix :-)

    logging.info('Registering %s to test %s', class_name, filename)
    test_class = type(class_name,          # class name
                      (OneIwyuTest,),      # superclass
                      # and methods.  f=filename is required for proper scoping
                      {'runTest': lambda self, f=filename: self.RunOneTest(f)})
    setattr(module, test_class.__name__, test_class)


if __name__ == '__main__':

  RegisterFilesForTesting()
  unittest.main()
