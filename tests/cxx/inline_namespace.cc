//===--- inline_namespace.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU never considers a decl inside an inline namespace
// forward-declarable, and that diagnostics never mention the inline namespace
// name (xyz::v1).

#include "tests/cxx/inline_namespace.h"

// IWYU: xyz::Foo is...*inline_namespace-i1.h
int Function(const xyz::Foo& foo) {
  // IWYU: xyz::Foo is...*inline_namespace-i1.h
  return foo.value;
}

/**** IWYU_SUMMARY

(tests/cxx/inline_namespace.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
