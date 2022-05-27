//===--- comment_style_long.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --comment_style=long -I .

// Test behavior is right with long comments.

#include "tests/cxx/comment_style-d1.h" // for bar

int main() {
  Foo::bar(1);
  // IWYU: Bar::foo is...*"tests/cxx/comment_style-i2.h"
  Bar::foo(2);
  return 0;
}

/**** IWYU_SUMMARY
tests/cxx/comment_style_long.cc should add these lines:
#include "tests/cxx/comment_style-i2.h"

tests/cxx/comment_style_long.cc should remove these lines:

The full include-list for tests/cxx/comment_style_long.cc:
#include "tests/cxx/comment_style-d1.h"  // for Foo::bar
#include "tests/cxx/comment_style-i2.h"  // for Bar::foo
***** IWYU_SUMMARY */
