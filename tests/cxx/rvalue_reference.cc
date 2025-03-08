//===--- rvalue_reference.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests handling rvalue references. Their pointed-to types are
// forward-declarable in general, as with lvalue references.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass needs a declaration
IndirectClass&& GetXValue();

// IWYU: IndirectClass needs a declaration
void Sink(IndirectClass&&) {
  // IWYU: IndirectClass needs a declaration
  IndirectClass&& rvalue_ref = GetXValue();
}

/**** IWYU_SUMMARY

tests/cxx/rvalue_reference.cc should add these lines:
class IndirectClass;

tests/cxx/rvalue_reference.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/rvalue_reference.cc:
class IndirectClass;

***** IWYU_SUMMARY */
