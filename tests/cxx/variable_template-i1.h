//===--- variable_template-i1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

template <typename T = IndirectClass>
T full_use_def_arg_provided;

template <typename T1, typename T2 = IndirectClass>
int both_args_used_def_provided = [] {
  T1 t1;
  T2 t2;
  return 1;
}();

using ProvidingRefAlias = IndirectClass&;

int GetInt();
