#!/usr/bin/env python

##===----------------------------------------------------------------------===
##
##                     The LLVM Compiler Infrastructure
##
## This file is distributed under the University of Illinois Open Source
## License. See LICENSE.TXT for details.
##
## Copyright Tom Rix 2019, all rights reserved.
## 
##===----------------------------------------------------------------------===

#
# Converts from an *.imp to legal json

import os.path
import sys
import re

ret = 0

def output_comment(s):
  comment_start = s.find('#')
  ss = s[comment_start+1:]
  ll = s.splitlines()
  for l in ll:
    # escape \ to \\
    m = l.replace('\\', '\\\\')
    # escape " to \"
    n = m.replace('\"', '\\"')
    print('  { \"comment\": \"' + n + '\" }, ')

def output_ref(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (ref:) (.*) }', l)
    print('  { \"ref\": \"' + m.group(2) + '\" }, ')

def output_include(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (include:) (.*), (.*), (.*), (.*) \] }', line)
    print('  { \"include\": ' + m.group(2) + ', \"' + m.group(3) + '\", ' + m.group(4) + ', \"' + m.group(5) + '\" ] }, ')

def output_include_q(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (include:) (.*), (.*), (.*), (.*) \] }', line)
    print('  { \"include\": ' + m.group(2) + ', ' + m.group(3) + ', ' + m.group(4) + ', ' + m.group(5) + ' ] }, ')

def output_symbol(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (symbol:) (.*), (.*), (.*), (.*) \] }', line)
    print('  { \"symbol\": ' + m.group(2) + ', \"' + m.group(3) + '\", ' + m.group(4) + ', \"' + m.group(5) + '\" ] }, ')

def output_symbol_q(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (symbol:) (.*), (.*), (.*), (.*) \] }', line)
    print('  { \"symbol\": ' + m.group(2) + ', ' + m.group(3) + ', ' + m.group(4) + ', ' + m.group(5) + ' ] }, ')
  
if os.path.isfile(sys.argv[1]):
  imp = sys.argv[1];
  f = open(imp)
  if f:
    lines = f.readlines()
    boiler_plate = False
    boiler_plate_list = list()
    for line in lines:
      if re.match("^(\s)*#", line):
        if boiler_plate:
          output_comment(line)
        else:
          boiler_plate_list.append(line)
      elif re.match('^\[.*$', line):
        sys.stdout.write(line)
        for bp in boiler_plate_list:
          output_comment(bp)
        boiler_plate = True
      elif re.match('^.*{ ref:', line):
        output_ref(line)
      elif re.match('^.*{ include: (.*), \"(.*)\", (.*), \"(.*)\" ]', line):
        output_include_q(line)
      elif re.match('^.*{ include:', line):
        output_include(line)
      elif re.match('^.*{ symbol: (.*), \"(.*)\", (.*), \"(.*)\" ]', line):
        output_symbol_q(line)
      elif re.match('^.*{ symbol:', line):
        output_symbol(line)
      elif re.match('^(\s)*$', line):
        # consume
        True
      else:
        sys.stdout.write(line)


