//===--- comment_style-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// this file exists to verify --comment_style works
#include "tests/cxx/comment_style-i2.h"  // Transitive include

namespace Foo {
  int bar(int x) {
    return x;
  }
};
