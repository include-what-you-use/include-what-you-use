//===--- include_cycle.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that we properly handle the case of include-cycles (header files
// including themselves, possibly indirectly)

#include "tests/cxx/include_cycle-d1.h"

IncludeCycleD1 d1;

// no output is expected because iwyu asserts on include-cycles

/**** IWYU_SUMMARY

***** IWYU_SUMMARY */
