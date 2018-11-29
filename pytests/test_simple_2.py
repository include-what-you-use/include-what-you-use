#!/usr/bin/env python
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
import harness

def test_simple_harness():
    harness.simple_run('simple.c')

test_simple_harness()
