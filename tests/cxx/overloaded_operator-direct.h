//===--- overloaded_operator-direct.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/overloaded_operator-indirect.h"

using ProvidingAlias = InnerOperatorStruct;
using ProvidingRefAlias = InnerOperatorStruct&;
