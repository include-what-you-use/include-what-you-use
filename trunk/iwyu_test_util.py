#!/usr/bin/python

##===--- iwyu_test_util.py - include-what-you-use test framework -----------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===-----------------------------------------------------------------------===##

"""Utilities for writing tests for IWYU."""

__author__ = 'wan@google.com (Zhanyong Wan)'

import difflib
import operator
import os
import re
import subprocess
import sys


_IWYU_PATH = '../../../../Debug+Asserts/bin/include-what-you-use'

# These are the warning/error lines that iwyu.cc produces when --verbose >= 3
_EXPECTED_DIAGNOSTICS_RE = re.compile(r'^(.*?):(\d+):.*//\s*IWYU:\s*(.*)$')
_ACTUAL_DIAGNOSTICS_RE = re.compile(r'^(.*?):(\d+):\d+:\s*'
                                    r'(?:warning|error):\s*(.*)$')

# This is the final summary output that iwyu.cc produces when --verbose >= 1
# The summary for a given source file should appear in that source file,
# surrounded by '/**** IWYU_SUMMARY' and '***** IWYU_SUMMARY */'.
_EXPECTED_SUMMARY_START_RE = re.compile(r'/\*+\s*IWYU_SUMMARY')
_EXPECTED_SUMMARY_END_RE = re.compile(r'\**\s*IWYU_SUMMARY\s*\*+/')
_ACTUAL_SUMMARY_START_RE = re.compile(r'^(.*?) should add these lines:$')
_ACTUAL_SUMMARY_END_RE = re.compile(r'^---$')
_ACTUAL_REMOVAL_LIST_START_RE = re.compile(r'.* should remove these lines:$')
_NODIFFS_RE = re.compile(r'^\((.*?) has correct #includes/fwd-decls\)$')

def _IsCppSource(file_path):
  return file_path.endswith('.h') or file_path.endswith('.cc')


def _GetAllCppFilesUnderDir(root_dir):
  cpp_files = []
  for (dir, _, files) in os.walk(root_dir):   # iterates over all dirs
    cpp_files += [os.path.join(dir, f) for f in files if _IsCppSource(f)]
  return cpp_files


def _GetCommandOutput(command):
  p = subprocess.Popen(command,
                       shell=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT,
                       close_fds=True)
  return p.stdout.readlines()


def _Permute(a_list):
  """Returns all permutations of a_list."""

  if len(a_list) <= 1:
    return [a_list]

  permutations = []
  for i in range(len(a_list)):
    head = a_list[i]
    tail = a_list[:i] + a_list[i + 1:]
    for p in _Permute(tail):
      permutations.append([head] + p)
  return permutations


def _GetExpectedDiagnosticRegexes(expected_diagnostic_specs):
  """Returns a map: source file location => list of regexes for that line."""

  # Maps a source file location to the warning/error message regex specified
  # on that line.
  spec_loc_to_regex = {}
  for line in expected_diagnostic_specs:
    m = _EXPECTED_DIAGNOSTICS_RE.match(line.strip())
    if m:
      path, line_num, regex = m.groups()
      if not regex:
        # Allow the regex to be omitted if we are uninterested in the
        # diagnostic message.
        regex = r'.*'
      spec_loc_to_regex[path, int(line_num)] = re.compile(regex)

  # Maps a source file line location to a list of regexes for diagnostics
  # that should be generated for that line.
  expected_diagnostic_regexes = {}
  regexes = []
  for loc in sorted(spec_loc_to_regex.keys()):
    regexes.append(spec_loc_to_regex[loc])
    # Do we have a spec on the next line?
    path, line_num = loc
    next_line_loc = path, line_num + 1
    if next_line_loc not in spec_loc_to_regex:
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
    for line in open(f):
      if _EXPECTED_SUMMARY_START_RE.match(line):
        in_summary = True
        expected_summaries[f] = []
      elif _EXPECTED_SUMMARY_END_RE.match(line):
        in_summary = False
      elif re.match(r'^\s*//', line):
        pass   # ignore comment lines
      elif in_summary:
        expected_summaries[f].append(line)

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

  # Newline-separated list of regexps and diagnostics
  regexes_str = '\n'.join([r.pattern for r in regexes])
  diagnostics_str = '\n'.join(diagnostics)

  # Is the number of diagnostics correct?
  regex_count = len(regexes)
  if len(diagnostics) != regex_count:
    return ['%s expecting %s diagnostics; actually had %s:\n%s\n' %
            (loc_str, regex_count, len(diagnostics), diagnostics_str)]

  if regex_count == 1:
    if regexes[0].search(diagnostics[0]):
      return []
    else:
      return ['%s actual diagnostic doesn\'t match expectation.\n'
              'Expected: regular expression "%s"\n'
              'Actual: %s\n' %
              (loc_str, regexes_str, diagnostics_str)]

  # There should be at least one permutation of the diagnostics that
  # matches the regexes.  We try all the permutations one-by-one.
  # This sounds bad, but is OK in practice as the list of diagnostics
  # on any given line is typically very small (1~2).
  for permutation in _Permute(diagnostics):
    for regex, diagnostic in zip(regexes, permutation):
      if not regex.search(diagnostic):
        # This permutation is not good.  Try the next.
        break
    else:
      # This permutation works!
      return []

  return [loc_str + ' no permutation of the actual diagnostics on this '
          'line matches the regex specs.\n'
          '--- Expected:\n%s\n--- Actual:\n%s\n---\n'
          % (regexes_str, diagnostics_str)]


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
      this_failure.next()     # read past the 'what files are this' header
      failures.append('\n')
      failures.append('Unexpected summary diffs for %s:\n' % loc)
      failures.extend(this_failure)
      failures.append('---\n')
    except StopIteration:
      pass                    # empty diff
  return failures


def TestIwyuOnRelativeFile(test_case, cc_file, cpp_files_to_check,
                           iwyu_flags=None):
  """Checks running IWYU on the given .cc file.

  Args:
    test_case: A googletest.TestCase instance.
    cc_file: The name of the file to test, relative to the current dir.
    cpp_files_to_check: A list of filenames for the files
              to check the diagnostics on, relative to the current dir.
    iwyu_flags: Extra command-line flags to pass to iwyu.
  """
  iwyu_flags = iwyu_flags or []  # Make sure iwyu_flags is a list.

  # Require verbose level 3 so that we can verify the individual diagnostics.
  iwyu_prefix = 'env IWYU_VERBOSE=3'

  # TODO(csilvers): verify that has exit-status 0.
  output = _GetCommandOutput('%s %s %s -I . %s' %
                             (iwyu_prefix, _IWYU_PATH, ' '.join(iwyu_flags),
                              cc_file))
  print ''.join(output)
  sys.stdout.flush()      # don't commingle this output with the failure output

  expected_diagnostics = _GetCommandOutput('grep -n -H "^ *// *IWYU" %s' %
                                           (' '.join(cpp_files_to_check)))
  failures = _CompareExpectedAndActualDiagnostics(
      _GetExpectedDiagnosticRegexes(expected_diagnostics),
      _GetActualDiagnostics(output))

  # Also figure out if the end-of-parsing suggestions match up.
  failures += _CompareExpectedAndActualSummaries(
      _GetExpectedSummaries(cpp_files_to_check),
      _GetActualSummaries(output))

  test_case.assert_(not failures, ''.join(failures))


# TODO(user): Move all tests using this function to the test directory
# harness, then get rid of it.
def TestIwyuOnFile(test_case, relative_test_dir, cc_file, iwyu_flags=None):
  """Checks running IWYU on the .cc file in the given directory."""

  TestIwyuOnRelativeFile(test_case,
                         os.path.join(relative_test_dir, cc_file),
                         _GetAllCppFilesUnderDir(relative_test_dir),
                         iwyu_flags)
