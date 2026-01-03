//===--- usearray.h - iwyu test -------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <array>

class UsesArray {
public:
  std::array<int, 10> theArray;
};
