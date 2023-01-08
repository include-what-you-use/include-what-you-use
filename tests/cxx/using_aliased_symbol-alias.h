//===--- using_aliased_symbol-alias.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/using_aliased_symbol-declare.h"

namespace ns2 {
using ns::symbol;
using ns::Typedef;
}  // namespace ns2
