#!/usr/bin/env python

##===--------- make_readme.py - generate README from Wiki sources ---------===##
#
#                      The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

from __future__ import print_function
import re
import sys
from datetime import datetime


_USAGE = """\
USAGE:
make_readme.py

Generate a Markdown README.md from selected content under docs/.

Example:
include-what-you-use$ ./make_readme.py > README.md
"""


def Heading():
  now = datetime.utcnow().replace(microsecond=0)

  buf = []
  buf.append('# Include What You Use #')
  buf.append('')
  buf.append('This README was generated on %s UTC.' % now.isoformat(' '))
  buf.append('')
  buf.append('For more in-depth documentation, see '
             'http://github.com/include-what-you-use/'
             'include-what-you-use/tree/master/docs.')
  buf.append('')
  buf.append('')

  return '\n'.join(buf)


def RewriteRelativeLinks(content):
  def Replace(m):
    if not m.group(2).startswith('http'):
        return '[' + m.group(1) + '](docs/' + m.group(2) + ')'
    return m.group(0)

  return re.sub(r'\[(.*?)\]\((.*?)\)', Replace, content)


def main(argv):
  if len(argv) != 1:
    print(_USAGE, file=sys.stderr)
    return 1

  print(Heading())

  with open('docs/InstructionsForUsers.md', 'r') as stream:
      print(RewriteRelativeLinks(stream.read()))

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
