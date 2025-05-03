//===--- b.h - iwyu test --------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef B_H
#define B_H

#ifndef oopsie
#define oopsie
#endif

#include "a.h"

struct SymbolB {
  struct SymbolA a;
};

#endif  // B_H
