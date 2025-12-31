//===--- comment_style_update_long.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --update_comments -Xiwyu --comment_style=long -I .

// Test that passing --update_comments respects comment style.

#include "tests/cxx/comment_style-d1.h"

int main() {
  // IWYU: Bar::foo(int) is...*"tests/cxx/comment_style-i2.h"
  Bar::foo(123);
  Foo::bar(456);
  return 0;
}

/**** IWYU_SUMMARY

tests/cxx/comment_style_update_long.cc should add these lines:
#include "tests/cxx/comment_style-i2.h"

tests/cxx/comment_style_update_long.cc should remove these lines:

The full include-list for tests/cxx/comment_style_update_long.cc:
#include "tests/cxx/comment_style-d1.h"  // for Foo::bar(int)
#include "tests/cxx/comment_style-i2.h"  // for Bar::foo(int)

***** IWYU_SUMMARY */
