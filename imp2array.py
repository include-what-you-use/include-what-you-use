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
# Converts from an *.imp to the equivelent c structure

import os.path
import sys
import re
import argparse

def output_comment(s):
  comment_start = s.find('#')
  ss = s[comment_start+1:]
  ws = ''
  if comment_start > 0:
    ws = s[:comment_start]
  ll = ss.splitlines()
  for l in ll:
    file_out.write(ws + '//' + l + '\n')

def output_ref(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (ref:) (.*) }', l)
    file_out.write('  { \"' + m.group(2) + '\", \"\", \"\", \"\" },\n')

def output_include(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (include:) \[\s*(.*), (.*), (.*), (.*) \] }', line)
    v0 = 'kPrivate'
    if re.match('.*ublic', m.group(3)):
      v0 = 'kPublic'
    v1 = 'kPublic'
    if re.match('.*ivate', m.group(5)):
      v1 = 'kPrivate'
    file_out.write('  { ' + m.group(2) + ', ' + v0 + ', ' + m.group(4) + ', ' + v1 + ' }, \n')

def output_symbol(s):
  ll = s.splitlines()
  for l in ll:
    m = re.match('^.*{ (symbol:) \[\s*(.*), (.*), (.*), (.*) \] }', line)
    v0 = 'kPrivate'
    if re.match('.*ublic', m.group(3)):
      v0 = 'kPublic'
    v1 = 'kPublic'
    if re.match('.*ivate', m.group(5)):
      v1 = 'kPrivate'
    file_out.write('  { ' + m.group(2) + ', ' + v0 + ', ' + m.group(4) + ', ' + v1 + ' }, \n')

# Assume failure.
ret = 1
# No error
err = False
# line number for reporting errors
line_number = 0
# Parse arguments.
parser = argparse.ArgumentParser(description='Converts *.imp to *.h')
parser.add_argument('-i', '--input', type=str, help='Input file', required=True)
parser.add_argument('-n', '--name', type=str, help='Array name', required=True)
parser.add_argument('-o', '--output', type=str, help='Output file', required=True)
parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')
args = parser.parse_args()

try:
  file_in = open(args.input, 'r')
except IOError as why:
  print('Error opening input file: ' + args.input + ', ' + why.strerror)

try:
  file_out = open(args.output, 'w')
except IOError as why:
  print('Error opening output file: ' + args.output + ', ' + why.strerror)
  
if file_in and file_out:
  lines = file_in.readlines()
  boiler_plate = False
  boiler_plate_list = list()
  for line in lines:
    if re.match("^(\s)*#", line):
      if boiler_plate:
        output_comment(line)
      else:
        boiler_plate_list.append(line)
    elif re.match('^\[.*$', line):
      for bp in boiler_plate_list:
        output_comment(bp)
      boiler_plate = True
      file_out.write('static const IncludeMapEntry ' + args.name + '[] = { \n')
    elif re.match('^].*$', line):
      file_out.write("};\n")
    elif re.match('^.*{ ref:', line):
      output_ref(line)
    elif re.match('^.*{ include:', line):
      output_include(line)
    elif re.match('^.*{ symbol:', line):
      output_symbol(line)
    elif re.match('^(\s)*$', line):
      # consume
      True
    else:
      if args.verbose:
        sys.stderr.write('Failed to match : ' + line)
      err = True
  if not err:
    ret = 0

if file_in:
  file_in.close()
if file_out:
  file_out.close()
  
# Return status.
sys.exit(ret)
