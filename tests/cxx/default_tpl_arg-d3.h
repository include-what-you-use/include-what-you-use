//===--- default_tpl_arg-d3.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file doesn't include "indirect.h" hence doesn't provide IndirectClass
// directly.

#include "tests/cxx/default_tpl_arg-d2.h"

using TplProvidingDefArg5Alias = TplProvidingDefArg5<>;
