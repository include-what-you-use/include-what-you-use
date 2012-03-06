//===--- header_near.h - test input file for iwyu -------------------------===//
//
// The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests including .h file by path relative to .cc file, not relative to current
// working directory.

class Foo {
 public:
  Foo();
};

/**** IWYU_SUMMARY

(tests/header_near.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
