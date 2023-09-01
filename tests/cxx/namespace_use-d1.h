//===--- namespace_use-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "namespace_use-i1.h"
#include "namespace_use-i2.h"

namespace i1_ns {

// Declare something extra in i1_ns to see if d1 or i1 is suggested
enum X {
  X1,
  X2,
};

}  // namespace i1_ns
