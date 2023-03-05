//===--- template_args_assoc.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/template_args_assoc.h"
#include "tests/cxx/direct.h"

// Test a template declared in the associated header and instantiated here.
// Issue #1181 describes a scenario where this led to the substituted template
// param being reported in the template declaration rather than the
// instantiation.

// IWYU_ARGS: -I .

void Test() {
  // IWYU: IndirectClass needs a declaration...*
  Template<IndirectClass> x;
}

/**** IWYU_SUMMARY

tests/cxx/template_args_assoc.cc should add these lines:
class IndirectClass;

tests/cxx/template_args_assoc.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/template_args_assoc.cc:
#include "tests/cxx/template_args_assoc.h"
class IndirectClass;

***** IWYU_SUMMARY */
