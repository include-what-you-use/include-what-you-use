//===--- template_member_functions-direct.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

template <typename T> struct TplUsingInDtor;

// This provides IndirectClass template argument.
using ProvidingTplUsingInDtor = TplUsingInDtor<IndirectClass>;

template <typename T = IndirectClass>
TplUsingInDtor<T> TplGetTplUsingInDtorDefArgProviding() {
  return TplUsingInDtor<T>();
}
