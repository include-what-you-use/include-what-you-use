#!/usr/bin/python

##===--------- make_readme.py - generate README from Wiki sources ---------===##
#
#                      The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

import os
import re
import sys
import glob
import textwrap
from datetime import datetime


_USAGE = """\
USAGE:
make_readme.py <path to wiki checkout>

Turn the entire IWYU Wiki into formatted text suitable for README.txt by
removing/transforming selected Wiki markup.

Example:
include-what-you-use$ ./make_readme.py ./wiki > README.txt
"""


# fixes all have the following traits:
# - take a line and return one or more lines as a single string
# - return None if the line is to be deleted
# - return an empty string if the line is to be left blank
def FixCodeBlock(line):
  if line.startswith('{{{') or line.startswith('}}}'):
    return ''

  return line


def FixNewLine(line):
  return line.strip('\r').strip('\n')


def FixDirectives(line):
  if line.startswith('#summary') or line.startswith('#labels'):
    return None

  return line


def FixLinks(line):
  return re.sub(R'\[.*?\s(.*?)\]', R'\1', line)


def FixBackticks(line):
  return line.replace('`', '')


def FixLinewrap(line):
  return textwrap.fill(line, 80)


def ApplyFixes(line, fixes):
  for fix in fixes:
    if line:
      line = fix(line)

  return line


def Unwikified(filename):
  fixes = [FixCodeBlock, FixDirectives, FixLinks, FixBackticks,
           FixNewLine, FixLinewrap]

  readme = []

  with open(filename, 'r') as stream:
    for line in stream.readlines():
      line = ApplyFixes(line, fixes)
      if line is not None:
        readme.append(line)

  return '\n'.join(readme)


def Heading():
  now = datetime.utcnow().replace(microsecond=0)

  buf = []
  buf.append('-' * 80)
  buf.append(' Include What You Use')
  buf.append('-' * 80)
  buf.append('')
  buf.append('This README was generated from the Wiki contents at')
  buf.append('http://code.google.com/p/include-what-you-use/w/ on %s UTC.'
                 % now.isoformat(' '))

  return '\n'.join(buf)


def ListWikiPages(wiki_pattern):
  def Prioritize(path):
    name = os.path.basename(path)
    prioritized = ['InstructionsForUsers.wiki',
                   'InstructionsForDevelopers.wiki',
                   'WhyIWYU.wiki']

    if name in prioritized:
      priority = prioritized.index(name)
    else:
      priority = len(prioritized)

    return priority

  excluded = []
  page_paths = [os.path.abspath(filename)
                for filename in glob.iglob(wiki_pattern)
                if not os.path.basename(filename) in excluded]

  return sorted(page_paths, key=Prioritize)


def main(argv):
  if len(argv) != 2:
    print(_USAGE)
    return 1

  print(Heading())

  path = argv[1]
  pattern = os.path.join(path, '*.wiki')
  for page_path in ListWikiPages(pattern):
    print(Unwikified(page_path))

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
