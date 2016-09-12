#!/usr/bin/env python

##===--- iwyu_test_util.py - include-what-you-use test framework -----------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===-----------------------------------------------------------------------===##

"""Utilities for writing tests for IWYU.

This script has been tested with python 2.7, 3.1.3 and 3.2.
In order to support all of these platforms there are a few unusual constructs:
 * print statements require parentheses
 * standard output must be decoded as utf-8
 * range() must be used in place of xrange()
 * _PortableNext() is used to obtain next iterator value

There is more detail on some of these issues at:
http://diveintopython3.org/porting-code-to-python-3-with-2to3.html
"""

__author__ = 'wan@google.com (Zhanyong Wan)'

import difflib
import operator
import os
import re
import subprocess
import sys

# These are the warning/error lines that iwyu.cc produces when --verbose >= 3
_EXPECTED_DIAGNOSTICS_RE = re.compile(r'^\s*//\s*IWYU:\s*(.*)$')
_ACTUAL_DIAGNOSTICS_RE = re.compile(r'^(.*?):(\d+):\d+:\s*'
                                    r'(?:warning|error|fatal error):\s*(.*)$')

# This is the final summary output that iwyu.cc produces when --verbose >= 1
# The summary for a given source file should appear in that source file,
# surrounded by '/**** IWYU_SUMMARY' and '***** IWYU_SUMMARY */'.
_EXPECTED_SUMMARY_START_RE = re.compile(r'/\*+\s*IWYU_SUMMARY')
_EXPECTED_SUMMARY_END_RE = re.compile(r'\**\s*IWYU_SUMMARY\s*\*+/')
_ACTUAL_SUMMARY_START_RE = re.compile(r'^(.*?) should add these lines:$')
_ACTUAL_SUMMARY_END_RE = re.compile(r'^---$')
_ACTUAL_REMOVAL_LIST_START_RE = re.compile(r'.* should remove these lines:$')
_NODIFFS_RE = re.compile(r'^\((.*?) has correct #includes/fwd-decls\)$')


def _PortableNext(iterator):
  if hasattr(iterator, 'next'):
    iterator.next()  # Python 2.4-2.6
  else:
    next(iterator)   # Python 3


def _Which(program, paths):
    """Searches specified paths for program."""
    if sys.platform == 'win32' and not program.lower().endswith('.exe'):
        program += '.exe'

    for path in paths:
        candidate = os.path.join(os.path.normpath(path), program)
        if os.path.isfile(candidate):
            return candidate

    return None


_IWYU_PATH = None
_SYSTEM_PATHS = [p.strip('"') for p in os.environ["PATH"].split(os.pathsep)]
_IWYU_PATHS = [
    '../../../../Debug+Asserts/bin',
    '../../../../Release+Asserts/bin',
    '../../../../Release/bin',
    '../../../../build/Debug+Asserts/bin',
    '../../../../build/Release+Asserts/bin',
    '../../../../build/Release/bin',
    # Linux/Mac OS X default out-of-tree paths.
    '../../../../../build/Debug+Asserts/bin',
    '../../../../../build/Release+Asserts/bin',
    '../../../../../build/Release/bin',
    # Windows default out-of-tree paths.
    '../../../../../build/bin/Debug',
    '../../../../../build/bin/Release',
    '../../../../../build/bin/MinSizeRel',
    '../../../../../build/bin/RelWithDebInfo',
    ]


def SetIwyuPath(iwyu_path):
  """Set the path to the IWYU executable under test.
  """
  global _IWYU_PATH
  _IWYU_PATH = iwyu_path


def _GetIwyuPath():
  """Returns the path to IWYU or raises IOError if it cannot be found."""
  global _IWYU_PATH

  if not _IWYU_PATH:
    iwyu_paths = _IWYU_PATHS + _SYSTEM_PATHS
    _IWYU_PATH = _Which('include-what-you-use', iwyu_paths)
    if not _IWYU_PATH:
      raise IOError('Failed to locate IWYU.\nSearched\n %s' %
                    '\n '.join(iwyu_paths))

  return _IWYU_PATH


def _ShellQuote(arg):
  if ' ' in arg:
    arg = '"' + arg + '"'
  return arg


def _GetCommandOutput(command):
  p = subprocess.Popen(command,
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
  stdout, _ = p.communicate()
  lines = stdout.decode("utf-8").splitlines(True)
  lines = [line.replace(os.linesep, '\n') for line in lines]
  return lines


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


def _StripCommentFromLine(line):
  """Removes the "// ..." comment at the end of the given line."""

  m = re.match(r'(.*)//', line)
  if m:
    return m.group(1).strip() + '\n'
  else:
    return line


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


def _VerifyDiagnosticsAtLoc(loc_str, regexes, diagnostics):
  """Verify the diagnostics at the given location; return a list of failures."""

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

  return ['%s %s' % (loc_str, message) for message in failure_messages]


def _CompareExpectedAndActualDiagnostics(expected_diagnostic_regexes,
                                         actual_diagnostics):
  """Verify that the diagnostics are as expected; return a list of failures."""

  failures = []
  for loc in sorted(set(actual_diagnostics.keys()) |
                    set(expected_diagnostic_regexes.keys())):
    # Find all regexes and actual diagnostics for the given location.
    regexes = expected_diagnostic_regexes.get(loc, [])
    diagnostics = actual_diagnostics.get(loc, [])
    failures += _VerifyDiagnosticsAtLoc('\n%s:%s:' % loc, regexes, diagnostics)

  return failures


def _CompareExpectedAndActualSummaries(expected_summaries, actual_summaries):
  """Verify that the summaries are as expected; return a list of failures."""

  failures = []
  for loc in sorted(set(actual_summaries.keys()) |
                    set(expected_summaries.keys())):
    this_failure = difflib.unified_diff(expected_summaries.get(loc, []),
                                        actual_summaries.get(loc, []))
    try:
      _PortableNext(this_failure)     # read past the 'what files are this' header
      failures.append('\n')
      failures.append('Unexpected summary diffs for %s:\n' % loc)
      failures.extend(this_failure)
      failures.append('---\n')
    except StopIteration:
      pass                    # empty diff
  return failures


def TestIwyuOnRelativeFile(test_case, cc_file, cpp_files_to_check,
                           iwyu_flags=None, clang_flags=None, verbose=False):
  """Checks running IWYU on the given .cc file.

  Args:
    test_case: A googletest.TestCase instance.
    cc_file: The name of the file to test, relative to the current dir.
    cpp_files_to_check: A list of filenames for the files
              to check the diagnostics on, relative to the current dir.
    iwyu_flags: Extra command-line flags to pass to iwyu.
    clang_flags: Extra command-line flags to pass to clang, for example
              "-std=c++11".
    verbose: Whether to display verbose output.
  """
  iwyu_flags = iwyu_flags or []  # Make sure iwyu_flags is a list.
  clang_flags = clang_flags or [] # Make sure this is a list

  # Require verbose level 3 so that we can verify the individual diagnostics.
  # We allow the level to be overriden by the IWYU_VERBOSE environment
  # variable, or by iwyu_flags, for easy debugging.  (We put the
  # envvar-based flag first, so user flags can override it later.)
  iwyu_flags = ['--verbose=%s' % os.getenv('IWYU_VERBOSE', '3')] + iwyu_flags

  # clang reads iwyu flags after the -Xiwyu clang flag: '-Xiwyu --verbose=6'
  iwyu_flags = ['-Xiwyu ' + flag for flag in iwyu_flags]

  # TODO(csilvers): verify that has exit-status 0.
  cmd = '%s %s %s %s' % (
    _ShellQuote(_GetIwyuPath()),
    ' '.join(iwyu_flags),
    ' '.join(clang_flags),
    cc_file)
  if verbose:
    print('>>> Running %s' % cmd)
  output = _GetCommandOutput(cmd)
  print(''.join(output))
  sys.stdout.flush()      # don't commingle this output with the failure output

  expected_diagnostics = _GetMatchingLines(
      _EXPECTED_DIAGNOSTICS_RE, cpp_files_to_check)
  failures = _CompareExpectedAndActualDiagnostics(
      _GetExpectedDiagnosticRegexes(expected_diagnostics),
      _GetActualDiagnostics(output))

  # Also figure out if the end-of-parsing suggestions match up.
  failures += _CompareExpectedAndActualSummaries(
      _GetExpectedSummaries(cpp_files_to_check),
      _GetActualSummaries(output))

  test_case.assertTrue(not failures, ''.join(failures))
