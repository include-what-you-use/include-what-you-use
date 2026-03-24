//===--- tpl.h - iwyu test ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "class.h"

template <typename T>
struct Tpl {
  typedef T Type;
  T GetClass();
};

struct Derived : Tpl<Class> {};

using Alias = Tpl<Class>;
