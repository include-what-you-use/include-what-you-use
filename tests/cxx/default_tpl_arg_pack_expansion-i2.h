//===--- default_tpl_arg_pack_expansion-i2.h - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/default_tpl_arg_pack_expansion-i1.h"

template <typename A, typename B, typename C = int>
struct Triple;
