//===--- badinc-d3.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D3_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D3_H_

#include <stdio.h>
#include "tests/badinc-i3.h"

namespace d3_namespace {
struct D3_Struct;
}

enum D3_Enum { D31, D32, D33 };

D3_Enum D3_Function(d3_namespace::D3_Struct* c) {
  return D31;
}

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D3_H_
