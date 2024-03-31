#!/usr/bin/env python3

##===--- iwyu_test_util.py - include-what-you-use test framework ----------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

"""Utilities for writing tests for IWYU.
"""

__author__ = 'wan@google.com (Zhanyong Wan)'

import difflib
import functools
import operator
import os
import re
import shlex
import shutil
import subprocess
import sys
import unittest

# These are the warning/error lines that iwyu.cc produces when --verbose >= 3
_EXPECTED_DIAGNOSTICS_RE = re.compile(r'^\s*// IWYU:\s*(.*)$')
_ACTUAL_DIAGNOSTICS_RE = re.compile(r'^(.*?):(\d+):\d+:\s*'
                                    r'(?:warning|error|fatal error):\s*(.*)$')

# Diagnostics without line/column number (e.g. from driver), e.g.:
# // IWYU~: argument unused during compilation
_EXPECTED_NOLOC_DIAGS_RE = re.compile(r'^\s*// IWYU~: (.*)$')
_ACTUAL_NOLOC_DIAGS_RE = re.compile(r'^(?:warning|error|fatal error):\s*(.*)$')

# This is the final summary output that iwyu.cc produces when --verbose >= 1
# The summary for a given source file should appear in that source file,
# surrounded by '/**** IWYU_SUMMARY' and '***** IWYU_SUMMARY */'.
# The leading summary line may also have an expected exit-code in parentheses
# after the summary marker: '/**** IWYU_SUMMARY(10)'.
_EXPECTED_SUMMARY_START_RE = re.compile(r'/\*+ IWYU_SUMMARY')
_EXPECTED_SUMMARY_EXIT_CODE_RE = re.compile(r'/\*+ IWYU_SUMMARY\((\d+)\)')
_EXPECTED_SUMMARY_END_RE = re.compile(r'\** IWYU_SUMMARY \*+/')
_ACTUAL_SUMMARY_START_RE = re.compile(r'^(.*?) should add these lines:$')
_ACTUAL_SUMMARY_END_RE = re.compile(r'^---$')
_ACTUAL_REMOVAL_LIST_START_RE = re.compile(r'.* should remove these lines:$')
_NODIFFS_RE = re.compile(r'^\((.*?) has correct #includes/fwd-decls\)$')

# This is an IWYU_ARGS line that specifies launch arguments for a test in its
# source file. Example:
# // IWYU_ARGS: -Xiwyu --mapping_file=... -I .
_IWYU_TEST_RUN_ARGS_RE = re.compile(r'^// IWYU_ARGS:\s(.*)$')

# Text matching a condition, either as part of IWYU_REQUIRES,
# IWYU_UNSUPPORTED or IWYU_XFAIL.
_IWYU_CONDITION = r'([a-z][a-z0-9_-]*)\(([^)]+)\)'

# This is a line that specifies prerequisites for a test (test will be skipped
# if they don't all hold). IWYU_REQUIRES specifies a condition that must be true
# for the test to execute. IWYU_UNSUPPORTED specifies a condition in which the
# test is invalid, and cannot be executed. Multiple IWYU_REQUIRES and
# IWYU_UNSUPPORTED lines can be specified. Examples:
# // IWYU_REQUIRES: feature(arg)
# // IWYU_UNSUPPORTED: feature(arg)
_IWYU_TEST_PREREQ_RE = re.compile(r'^// IWYU_(REQUIRES|UNSUPPORTED): ' +
                                  _IWYU_CONDITION + r'$')

# This is a line that specifies conditions under which a test is expected to
# fail.
# // IWYU_XFAIL: feature(arg)
_IWYU_TEST_XFAIL_RE = re.compile(r'^// IWYU_XFAIL(?:: ' +
                                 _IWYU_CONDITION + r')?$')


def cache(fn):
  """ functools has a cache decorator starting with Python 3.9. Support 3.2+ with
  a local alias.
  """
  return functools.lru_cache(maxsize=None)(fn)


class Features:
  """ Implements feature support functions for prerequisites and
      expected failures. """

  # Parses the output of 'include-what-you-use -print-targets'.
  SUPPORTED_TARGET_RE = re.compile(r'^\s+([a-z0-9_-]+)\s* - .*$')

  # Parses the output of 'include-what-you-use -dM -E'.
  PREPROCESSOR_MACRO_RE = re.compile(r'^#define ([A-z_][A-z0-9_]*)')

  def __init__(self):
    # Maps all known features names to prerequisite predicates.
    self.known = {
      'has-target': Features.HasTarget,
      'uses-cstdlib': Features.UsesCstdlib,
      'uses-cxxstdlib': Features.UsesCXXstdlib,
    }

  @staticmethod
  def _GetPreprocessorMacros(lang, header):
    iwyu = _GetIwyuPath()
    src = '#include %s' % header

    cmd = [iwyu, '-dM', '-E', '-x', lang]
    # Take into account extra arguments specified in IWYU_EXTRA_ARGS -
    # the user may be trying to test with different includes, standard
    # libraries or whatever.
    #
    # For feature tests we _don't_ take into account test-specific
    # IWYU_ARGS.
    #  * the argument could be the thing that breaks things
    #  * the feature result should not be test specific
    cmd += _GetExtraArgs()
    cmd += ['-']

    result = subprocess.run(cmd, capture_output=True, input=src, text=True)

    macros = []
    for line in result.stdout.splitlines():
      m = Features.PREPROCESSOR_MACRO_RE.match(line)
      if m:
        macros.append(m.group(1))

    return macros

  @staticmethod
  @cache
  def _GetCXXPreprocessorMacros():
    return Features._GetPreprocessorMacros('c++', '<cstdlib>')

  @staticmethod
  @cache
  def _GetCPreprocessorMacros():
    return Features._GetPreprocessorMacros('c', '<stdlib.h>')

  @staticmethod
  def HasTarget(target):
    iwyu = _GetIwyuPath()
    result = subprocess.run([iwyu, '-print-targets'],
                            capture_output=True,
                            text=True)
    supported = []
    for line in result.stdout.splitlines():
      m = Features.SUPPORTED_TARGET_RE.match(line)
      if m:
        supported.append(m.group(1))
    return target in supported

  @staticmethod
  def UsesCXXstdlib(lib):
    id_macros = {
      'libcxx': ['_LIBCPP_VERSION'],
      'libstdcxx': ['__GLIBCXX__'],
      # https://github.com/microsoft/STL/wiki/Macro-_MSVC_STL_UPDATE
      'msvc': ['_MSVC_STL_UPDATE'],
    }
    macros = id_macros.get(lib)
    if macros is None:
      raise SyntaxError('Unknown C++ standard library: %s' % lib)

    return all(macro in Features._GetCXXPreprocessorMacros() for macro in macros)

  @staticmethod
  def UsesCstdlib(lib):
    id_macros = {
      'glibc': ['__GLIBC__'],
      'llvmlibc': ['__LLVM_LIBC__'],

      # On MacOS sys/cdefs.h defines a bunch of macros. Use
      # __DARWIN_C_LEVEL to identify the MacOS C library.
      # https://opensource.apple.com/source/xnu/xnu-7195.81.3/bsd/sys/cdefs.h.auto.html
      'macos': ['__DARWIN_C_LEVEL'],

      # _MSC_VER identifies the Microsoft compiler rather than the
      # standard library (MSVCRT or UCRT). clang will also define this
      # for MS targets.
      'msvcrt': ['_MSC_VER'],
      'newlib': ['__NEWLIB__'],
      'uclibc': ['__UCLIBC__'],
    }
    macros = id_macros.get(lib)
    if macros is None:
      raise SyntaxError('Unknown C standard library: %s' % lib)

    # TODO: This needs refining to ensure it correctly identifies said libraries
    return all(macro in Features._GetCPreprocessorMacros() for macro in macros)

_FEATURES = Features()


_IWYU_PATH = shutil.which('include-what-you-use')


def SetIwyuPath(iwyu_path):
  """Set the path to the IWYU executable under test.
  """
  global _IWYU_PATH
  _IWYU_PATH = iwyu_path


def _GetIwyuPath():
  """Returns the path to IWYU or raises IOError if it cannot be found."""
  if not _IWYU_PATH:
    raise IOError('\'include-what-you-use\' not found in PATH')
  return _IWYU_PATH


def _GetCommandOutput(command):
  p = subprocess.Popen(command,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
  stdout, _ = p.communicate()
  lines = stdout.decode("utf-8").splitlines(True)
  lines = [line.replace(os.linesep, '\n') for line in lines]
  return p.returncode, lines


def _GetExtraArgs():
  """Retrieve IWYU_EXTRA_ARGS from the environment and return as a list"""
  env_iwyu_extra_args = os.getenv('IWYU_EXTRA_ARGS')
  if env_iwyu_extra_args:
    return shlex.split(env_iwyu_extra_args)
  return []


def _GetMatchingLines(regex, file_names):
  """Returns a map: file location => string matching `regex`.

  File location is a tuple (file_name, line number starting from 1)."""

  loc_to_line = {}
  for file_name in file_names:
    with open(file_name) as fileobj:
      for line_num, line in enumerate(fileobj):
        m = regex.match(line)
        if m:
          loc_to_line[file_name, line_num + 1] = m.group()
  return loc_to_line


def _GetExpectedDiagnosticRegexes(spec_loc_to_line):
  """Returns a map: source file location => list of regexes for that line."""

  # Maps a source file line location to a list of regexes for diagnostics
  # that should be generated for that line.
  expected_diagnostic_regexes = {}
  regexes = []
  for loc in sorted(spec_loc_to_line.keys()):
    line = spec_loc_to_line[loc]
    m = _EXPECTED_DIAGNOSTICS_RE.match(line.strip())
    assert m is not None, "Input should contain only matching lines."
    regex = m.group(1)
    if not regex:
      # Allow the regex to be omitted if we are uninterested in the
      # diagnostic message.
      regex = r'.*'
    regexes.append(re.compile(regex))
    # Do we have a spec on the next line?
    path, line_num = loc
    next_line_loc = path, line_num + 1
    if next_line_loc not in spec_loc_to_line:
      expected_diagnostic_regexes[next_line_loc] = regexes
      regexes = []

  return expected_diagnostic_regexes


def _GetActualDiagnostics(actual_output):
  """Returns a map: source file location => list of diagnostics on that line.

  The elements of the list are unique and sorted."""

  actual_diagnostics = {}
  for line in actual_output:
    m = _ACTUAL_DIAGNOSTICS_RE.match(line.strip())
    if m:
      path, line_num, message = m.groups()
      loc = path, int(line_num)
      actual_diagnostics[loc] = actual_diagnostics.get(loc, []) + [message]

  locs = actual_diagnostics.keys()
  for loc in locs:
    actual_diagnostics[loc] = sorted(set(actual_diagnostics[loc]))

  return actual_diagnostics


def _GetExpectedNoLocDiagnosticRegexes(filename):
  """Returns a list of regexes for the file."""
  expected = []
  with open(filename, 'r') as fileobj:
    for line in fileobj:
      m = _EXPECTED_NOLOC_DIAGS_RE.match(line.strip())
      if m:
        expected.append(re.compile(m.group(1)))
  return expected


def _GetActualNoLocDiagnostics(actual_output):
  """Returns a list of diagnostic messages."""
  actual_diagnostics = []
  for line in actual_output:
    m = _ACTUAL_NOLOC_DIAGS_RE.match(line.strip())
    if m:
      actual_diagnostics.append(m.group(1))
  return actual_diagnostics


def _StripCommentFromLine(line):
  """Removes the "// ..." comment at the end of the given line."""
  return re.sub(r'\s*//.*$', '', line)


def _NormalizeSummaryLineNumbers(line):
  """Replaces the comment '// lines <number>-<number>' with '// lines XX-YY'.

  Because line numbers in the source code often change, it's a pain to
  keep the '// lines <number>-<number>' comments accurate in our
  'golden' output.  Instead, we normalize these iwyu comments to just
  say how many line numbers are listed by mapping the output to
  '// lines XX-XX' (for one-line spans) or '// lines XX-XX+<number>'.
  For instance, '// lines 12-12' would map to '// lines XX-XX', while
  '// lines 12-14' would map to '//lines XX-XX+2'.

  Arguments:
    line: the line to be normalized.

  Returns:
    A new line with the '// lines' comment, if any, normalized as
    described above.  If no '// lines' comment is present, returns
    the original line.
  """
  m = re.search('// lines ([0-9]+)-([0-9]+)', line)
  if not m:
    return line
  if m.group(1) == m.group(2):
    return line[:m.start()] + '// lines XX-XX\n'
  else:
    num_lines = int(m.group(2)) - int(m.group(1))
    return line[:m.start()] + '// lines XX-XX+%d\n' % num_lines


def _NormalizeSummaryLine(line):
  """Alphabetically sorts the symbols in the '// for XXX, YYY, ZZZ' comments.

  Most iwyu summary lines have the form
     #include <foo.h>   // for XXX, YYY, ZZZ
  XXX, YYY, ZZZ are symbols that this file uses from foo.h.  They are
  sorted in frequency order, but that changes so often as the test is
  augmented, that it's impractical to test.  We just sort the symbols
  alphabetically and compare that way.  This means we never test the
  frequency ordering here, but that's a small price to pay for easier
  testing development.

  We also always move the '// for' comment to be exactly two spaces
  after the '#include' text.  Again, this means we don't test the
  indenting correctly (though iwyu_output_test.cc does), but allows us
  to rename filenames without having to reformat each test.  This is
  particularly important when opensourcing, since the filenames will
  be different in opensource-land than they are inside google.

  Arguments:
    line: one line of the summary output

  Returns:
    A normalized form of 'line', with the 'why' symbols sorted and
    whitespace before the 'why' comment collapsed.
  """
  m = re.match(r'(.*?)\s*  // for (.*)', line)
  if not m:
    return line
  symbols = m.group(2).strip().split(', ')
  symbols.sort()
  return '%s  // for %s\n' % (m.group(1), ', '.join(symbols))


def _GetExpectedSummaries(files):
  """Returns a map: source file => list of iwyu summary lines."""

  expected_summaries = {}
  for f in files:
    in_summary = False
    fh = open(f)
    for line in fh:
      if _EXPECTED_SUMMARY_START_RE.match(line):
        in_summary = True
        expected_summaries[f] = []
      elif _EXPECTED_SUMMARY_END_RE.match(line):
        in_summary = False
      elif re.match(r'^\s*//', line):
        pass   # ignore comment lines
      elif in_summary:
        expected_summaries[f].append(line)
    fh.close()

  # Get rid of blank lines at the beginning and end of the each summary.
  for loc in expected_summaries:
    while expected_summaries[loc] and expected_summaries[loc][-1] == '\n':
      expected_summaries[loc].pop()
    while expected_summaries[loc] and expected_summaries[loc][0] == '\n':
      expected_summaries[loc].pop(0)

  return expected_summaries


def _GetExpectedExitCode(main_file):
  with open(main_file, 'r') as fh:
    for line in fh:
      m = _EXPECTED_SUMMARY_EXIT_CODE_RE.match(line)
      if m:
        res = int(m.group(1))
        return res
  return None


def _GetActualSummaries(output):
  """Returns a map: source file => list of iwyu summary lines."""

  actual_summaries = {}
  file_being_summarized = None
  in_addition_section = False  # Are we in the "should add these lines" section?
  for line in output:
    # For files with no diffs, we print a different (one-line) summary.
    m = _NODIFFS_RE.match(line)
    if m:
      actual_summaries[m.group(1)] = [line]
      continue

    m = _ACTUAL_SUMMARY_START_RE.match(line)
    if m:
      file_being_summarized = m.group(1)
      in_addition_section = True
      actual_summaries[file_being_summarized] = [line]
    elif _ACTUAL_SUMMARY_END_RE.match(line):
      file_being_summarized = None
    elif file_being_summarized:
      if _ACTUAL_REMOVAL_LIST_START_RE.match(line):
        in_addition_section = False
      # Replace any line numbers in comments with something more stable.
      line = _NormalizeSummaryLineNumbers(line)
      if in_addition_section:
        # Each #include in the "should add" list will appear later in
        # the full include list.  There's no need to verify its symbol
        # list twice.  Therefore we remove the symbol list here for
        # easy test maintenance.
        line = _StripCommentFromLine(line)
      else:
        line = _NormalizeSummaryLine(line)
      actual_summaries[file_being_summarized].append(line)

  return actual_summaries


def _VerifyDiagnostics(regexes, diagnostics, loc_str=''):
  """Verify the diagnostics; return a list of failures."""
  # Find out which regexes match a diagnostic and vice versa.
  matching_regexes = [[] for unused_i in range(len(diagnostics))]
  matched_diagnostics = [[] for unused_i in range(len(regexes))]
  for (r_index, regex) in enumerate(regexes):
    for (d_index, diagnostic) in enumerate(diagnostics):
      if regex.search(diagnostic):
        matching_regexes[d_index].append(r_index)
        matched_diagnostics[r_index].append(d_index)

  failure_messages = []

  # Collect unmatched diagnostics and multiply matched diagnostics.
  for (d_index, r_indexes) in enumerate(matching_regexes):
    if not r_indexes:
      failure_messages.append('Unexpected diagnostic:\n%s\n'
                              % diagnostics[d_index])
    elif len(r_indexes) > 1:
      failure_messages.append(
          'The diagnostic message:\n%s\n'
          'matches multiple regexes:\n%s'
          % (diagnostics[d_index],
             '\n'.join([regexes[r_index].pattern for r_index in r_indexes])))

  # Collect unmatched regexes and regexes with multiple matches.
  for (r_index, d_indexes) in enumerate(matched_diagnostics):
    if not d_indexes:
      failure_messages.append('Unmatched regex:\n%s\n'
                              % regexes[r_index].pattern)
    elif len(d_indexes) > 1:
      failure_messages.append(
          'The regex:\n%s\n'
          'matches multiple diagnostics:\n%s'
          % (regexes[r_index].pattern,
             '\n'.join([diagnostics[d_index] for d_index in d_indexes])))

  return ['\n%s%s' % (loc_str, message) for message in failure_messages]


def _CompareExpectedAndActualDiagnostics(expected_diagnostic_regexes,
                                         actual_diagnostics):
  """Verify that the diagnostics are as expected; return a list of failures."""

  failures = []
  for loc in sorted(set(actual_diagnostics.keys()) |
                    set(expected_diagnostic_regexes.keys())):
    # Find all regexes and actual diagnostics for the given location.
    regexes = expected_diagnostic_regexes.get(loc, [])
    diagnostics = actual_diagnostics.get(loc, [])
    failures += _VerifyDiagnostics(regexes, diagnostics, '%s:%s: ' % loc)

  return failures


def _CompareExpectedAndActualNoLocDiagnostics(expected_regexes,
                                              actual_diagnostics):
  """Verify that every regex in expected matches a line in actual; return a list
  of failures"""
  return _VerifyDiagnostics(expected_regexes, actual_diagnostics)


def _CompareExpectedAndActualSummaries(expected_summaries, actual_summaries):
  """Verify that the summaries are as expected; return a list of failures."""

  failures = []
  for loc in sorted(set(actual_summaries.keys()) |
                    set(expected_summaries.keys())):
    this_failure = difflib.unified_diff(expected_summaries.get(loc, []),
                                        actual_summaries.get(loc, []))
    try:
      next(this_failure)     # read past the 'what files are this' header
      failures.append('\n')
      failures.append('Unexpected summary diffs for %s:\n' % loc)
      failures.extend(this_failure)
      failures.append('---\n')
    except StopIteration:
      pass                    # empty diff
  return failures


def _GetLaunchArguments(cc_file):
  """Gets IWYU launch arguments for a source file from its contents."""
  args = ''
  with open(cc_file) as it:
    # Find the first '// IWYU_ARGS: ' line.
    for lineno, line in enumerate(it):
      m = _IWYU_TEST_RUN_ARGS_RE.match(line)
      if m:
        args = m.group(1)
        break

    for line in it:
      # Consume all comment lines until we hit one that doesn't have a
      # multi-line continuation.
      if not line.startswith('// ') or not args.endswith('\\'):
         break
      line = line[3:].strip()
      args = args[:-1] + ' ' + line

    if args.endswith('\\'):
      raise SyntaxError('%s:%s syntax error in multiline IWYU_ARGS' %
          (cc_file, lineno))

  return shlex.split(args)


def _ParsePrerequisites(cc_file):
  """ Parses test prerequisites out of cc_file. """
  prerequisites = []
  with open(cc_file) as it:
    for lineno, line in enumerate(it):
      m = _IWYU_TEST_PREREQ_RE.match(line)
      if not m:
        continue

      directive = m.group(1)
      feature = m.group(2)
      value = m.group(3)

      feature_pred = _FEATURES.known.get(feature)
      if not feature_pred:
        raise SyntaxError('%s:%s IWYU_%s: unsupported feature: %s' %
                          (cc_file, lineno, directive, feature))

      if directive == 'REQUIRES':
        prerequisite = lambda arg=value: feature_pred(arg)
      elif directive == 'UNSUPPORTED':
        prerequisite = lambda arg=value: not feature_pred(arg)
      else:
        assert False, ('_IWYU_TEST_PREREQ_RE should not have matched: %s' %
                       directive)

      desc = '%s: %s(%s)' % (directive, feature, value)
      prerequisites.append((prerequisite, desc))
    return prerequisites


def _CheckPrerequisites(cc_file):
  """ Raises a SkipTest exception if any prerequisites in cc_file fail. """
  prerequisites = _ParsePrerequisites(cc_file)

  failed = []
  for prerequisite, desc in prerequisites:
    if not prerequisite():
      failed.append(desc)

  if failed:
    raise unittest.SkipTest(', '.join(failed))


def _ParseExpectedFailures(cc_file):
  """ Parses expected failure conditions out of cc_file. """
  conditions = []
  with open(cc_file) as it:
    for lineno, line in enumerate(it):
      m = _IWYU_TEST_XFAIL_RE.match(line)
      if not m:
        continue

      feature = m.group(1)
      value = m.group(2)

      if feature is None:
        # Bare IWYU_XFAIL, condition is always True
        condition = lambda: True
      else:
        feature_cond = _FEATURES.known.get(feature)
        if not feature_cond:
          raise SyntaxError('%s:%s IWYU_XFAIL: unsupported feature: %s' %
                            (cc_file, lineno, feature))

        condition = lambda arg=value: feature_cond(arg)

      conditions.append(condition)
    return conditions


def IsTestExpectedToFail(cc_file):
  """ Checks if a test is expected to fail. """
  xfail_conditions = _ParseExpectedFailures(cc_file)
  return any(condition() for condition in xfail_conditions)


def TestIwyuOnRelativeFile(cc_file, cpp_files_to_check, verbose=False):
  """Checks running IWYU on the given .cc file.

  Args:
    cc_file: The name of the file to test, relative to the current dir.
    cpp_files_to_check: A list of filenames for the files
              to check the diagnostics on, relative to the current dir.
    verbose: Whether to display verbose output.
  """
  # Parse and check IWYU_{REQUIRES,UNSUPPORTED}
  _CheckPrerequisites(cc_file)

  cmd = [_GetIwyuPath()]
  # Require verbose level 3 so that we can verify the individual diagnostics.
  # We allow the level to be overriden by
  # * IWYU_ARGS comment in a test file
  # * IWYU_VERBOSE environment variable
  cmd += ['-Xiwyu', '--verbose=3']
  cmd += _GetLaunchArguments(cc_file)
  env_verbose_level = os.getenv('IWYU_VERBOSE')
  if env_verbose_level:
    cmd += ['-Xiwyu', '--verbose=' + env_verbose_level]
  cmd += _GetExtraArgs()
  cmd += [cc_file]

  if verbose:
    print('>>> Running %s' % shlex.join(cmd))
  exit_code, output = _GetCommandOutput(cmd)
  print(''.join(output))
  sys.stdout.flush()      # don't commingle this output with the failure output

  # Verify exit code if requested
  expected_exit_code = _GetExpectedExitCode(cc_file)
  if expected_exit_code is not None and exit_code != expected_exit_code:
    raise AssertionError('Unexpected exit code, wanted %d, was %d' %
                         (expected_exit_code, exit_code))

  # Check diagnostics without location (from driver)
  failures = _CompareExpectedAndActualNoLocDiagnostics(
      _GetExpectedNoLocDiagnosticRegexes(cc_file),
      _GetActualNoLocDiagnostics(output))

  # Check IWYU diagnostics
  expected_diagnostics = _GetMatchingLines(
      _EXPECTED_DIAGNOSTICS_RE, cpp_files_to_check)
  failures += _CompareExpectedAndActualDiagnostics(
      _GetExpectedDiagnosticRegexes(expected_diagnostics),
      _GetActualDiagnostics(output))

  # Also figure out if the end-of-parsing suggestions match up.
  failures += _CompareExpectedAndActualSummaries(
      _GetExpectedSummaries(cpp_files_to_check),
      _GetActualSummaries(output))

  if failures:
    raise AssertionError(''.join(failures))
