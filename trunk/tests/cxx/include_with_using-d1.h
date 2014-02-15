//===--- include_with_using-d1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/include_with_using-i1.h"

namespace ns {
class PtrInNs {};
}

using ns::PtrInNs;
