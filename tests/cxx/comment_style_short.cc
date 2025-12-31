//===--- comment_style_short.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --comment_style=short -I .

// Test that --comment_style=short adds short comments.

#include "tests/cxx/comment_style-d1.h" // some Comment

int main() {
  Foo::bar(1);
  // IWYU: Bar::foo(int) is...*"tests/cxx/comment_style-i2.h"
  Bar::foo(2);
  return 0;
}

/**** IWYU_SUMMARY
tests/cxx/comment_style_short.cc should add these lines:
#include "tests/cxx/comment_style-i2.h"

tests/cxx/comment_style_short.cc should remove these lines:

The full include-list for tests/cxx/comment_style_short.cc:
#include "tests/cxx/comment_style-d1.h"  // for bar
#include "tests/cxx/comment_style-i2.h"  // for foo
***** IWYU_SUMMARY */
