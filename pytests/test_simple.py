#!/usr/bin/env python
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
import difflib
import pytest
import os
import shutil
import subprocess
import tempfile

here = os.path.dirname(os.path.realpath(__file__))
test_file = 'simple.c'
test_path = here + os.sep + test_file
input =  test_path + '.input'
iwyu = test_path + '.iwyu'
diff = test_path + '.diff'
fix_includes = here + os.sep + '..' + os.sep + 'fix_includes.py'
def test_simple():
    shutil.copyfile(input, test_path)
    fix_includes_input = open(iwyu)
    tout = tempfile.TemporaryFile();
    terr = tempfile.TemporaryFile();
    o = subprocess.call([fix_includes], stdin=fix_includes_input, stdout=tout, stderr=terr)
    f = open(input)
    a = f.read().splitlines()
    f.close()
    f = open(test_path)
    b = f.read().splitlines()
    f.close()
    f = open(diff)
    e = f.read().splitlines()
    f.close()

    #d = ""
    ud = difflib.unified_diff(a, b, fromfile=test_file + '.input', tofile=test_file)
    for dd in ud:
        dd = dd.strip()
        # d = d + dd  + '\n'

    #print d

    os.remove(test_path)

    assert(all([i == j for i, j in zip(ud, e)]))
    
test_simple()
