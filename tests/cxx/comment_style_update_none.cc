//===--- comment_style_update_none.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --update_comments -Xiwyu --comment_style=none -I .

// Test that --update_comments respects comment style.

#include "tests/cxx/comment_style-d1.h" // for foo, bar

int main() {
  // IWYU: Bar::foo is...*"tests/cxx/comment_style-i2.h"
  Bar::foo(123);
  Foo::bar(456);
  return 0;
}

/**** IWYU_SUMMARY

tests/cxx/comment_style_update_none.cc should add these lines:
#include "tests/cxx/comment_style-i2.h"

tests/cxx/comment_style_update_none.cc should remove these lines:

The full include-list for tests/cxx/comment_style_update_none.cc:
#include "tests/cxx/comment_style-d1.h"
#include "tests/cxx/comment_style-i2.h"

***** IWYU_SUMMARY */
