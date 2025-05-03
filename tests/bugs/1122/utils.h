//===--- utils.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef UTILS_H
#define UTILS_H

#include "my_int.h"

template <typename T>
constexpr int TimesTen(T n) {
  return MakeMyInt(n) * 10;
}

#endif  // UTILS_H
