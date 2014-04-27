//===--- fwd_decl_static_member.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that static data members can be forward declared even if they are not
// pointers or references.

#include "tests/cxx/indirect.h"

class Outer {
 private:
  static IndirectClass forward_declarable;
};

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_static_member.cc should add these lines:
class IndirectClass;

tests/cxx/fwd_decl_static_member.cc should remove these lines:
- #include "tests/cxx/indirect.h"  // lines XX-XX

The full include-list for tests/cxx/fwd_decl_static_member.cc:
class IndirectClass;

***** IWYU_SUMMARY */
