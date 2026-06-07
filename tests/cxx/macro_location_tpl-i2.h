//===--- macro_location_tpl-i2.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location_tpl-i1.h"
#include "tests/cxx/macro_location_tpl-i3.h"
#include "tests/cxx/macro_location_tpl-i4.h"

SPECIALIZE_TPL(Struct)

DEFINE_FN1_TAKING(Struct)

DEFINE_TPL_FN_AND_FN2

#define CALL_FN2(Arg) (Fn2(Arg))

DEFINE_DEF_ARG_1

DEFINE_USE_PROVIDED_DEF_ARG_2
