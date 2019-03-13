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

ret = 0

if os.path.isfile(sys.argv[1]):
  imp = sys.argv[1];
  f = open(imp)
  if f:
    if sys.argv[2]:
      print 'static const IncludeMapEntry ' + sys.argv[2] + '[] = \n'
    lines = f.readlines()
    boiler_plate = False
    for line in lines:
      nl = line
      if re.search("#", line):
        # replace # comments with //
        if boiler_plate:
          nl = line.replace("#", "//")
        else:
          nl = ""
      elif re.match('^]$', line):
        nl = "};\n"
      elif re.match('^\[$', line):
        nl = "{\n"
        boiler_plate = True
      else:
        nl = ""
        # From
        #  { include: [ "<bits/a.out.h>", private, "<a.out.h>", public ] },
        # To
        #  { "<bits/a.out.h>", kPrivate, "<a.out.h>", kPublic },
        m = re.match('.*: \[ \"(.*)\", private, \"(.*)\", public', line)
        if m:
          priv=m.group(1)
          pub=m.group(2)
          nl = "  { \"" + priv + "\", kPrivate, \"" + pub + "\", kPublic },\n"

      sys.stdout.write(nl)


