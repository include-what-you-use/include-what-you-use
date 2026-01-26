//===--- comment_style_none.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --comment_style=none -I .

// Test that --comment_style=none adds no comments.

#include "tests/cxx/comment_style-d1.h"

int main() {
  Foo::bar(1);
  // IWYU: Bar::foo(int) is...*"tests/cxx/comment_style-i2.h"
  Bar::foo(2);
  return 0;
}

/**** IWYU_SUMMARY
tests/cxx/comment_style_none.cc should add these lines:
#include "tests/cxx/comment_style-i2.h"

tests/cxx/comment_style_none.cc should remove these lines:

The full include-list for tests/cxx/comment_style_none.cc:
#include "tests/cxx/comment_style-d1.h"
#include "tests/cxx/comment_style-i2.h"
***** IWYU_SUMMARY */
