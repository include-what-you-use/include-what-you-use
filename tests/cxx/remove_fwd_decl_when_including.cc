//===--- remove_fwd_decl_when_including.cc - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// This tests the following behavior: when we need to #include a
// file to get the full type of Foo (here, Foo == IndirectClass),
// but a forward-declare will also suffice for another use of Foo,
// *and* we forward-declare foo, make sure we get rid of the
// forward-declare, which is superseded by the #include.
//
// In other words, we should never have the case where we both
// #include a class, and forward-declare the same class, in a file.

#include "tests/cxx/direct.h"

class IndirectClass;

int main() {
  IndirectClass* p;
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass i;
}

/**** IWYU_SUMMARY

tests/cxx/remove_fwd_decl_when_including.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/remove_fwd_decl_when_including.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/remove_fwd_decl_when_including.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
