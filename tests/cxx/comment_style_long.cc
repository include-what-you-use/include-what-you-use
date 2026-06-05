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

// IWYU: Bar::FullAndFwdDeclUse is...*comment_style-i2.h
Bar::FullAndFwdDeclUse full_use;
// IWYU: Bar::FullAndFwdDeclUse needs a declaration
Bar::FullAndFwdDeclUse* fwd_decl_use1;
// IWYU: Bar::FwdDeclUse needs a declaration
Bar::FwdDeclUse* fwd_decl_use2;

int main() {
  Foo::bar(1);
  // IWYU: Bar::foo(int) is...*"tests/cxx/comment_style-i2.h"
  Bar::foo(2);
  return 0;
}

/**** IWYU_SUMMARY
tests/cxx/comment_style_long.cc should add these lines:
#include "tests/cxx/comment_style-i2.h"

tests/cxx/comment_style_long.cc should remove these lines:

The full include-list for tests/cxx/comment_style_long.cc:
#include "tests/cxx/comment_style-d1.h"  // for Foo::bar(int)
#include "tests/cxx/comment_style-i2.h"  // for Bar::FullAndFwdDeclUse, Bar::FwdDeclUse (ptr only), Bar::foo(int)
***** IWYU_SUMMARY */
