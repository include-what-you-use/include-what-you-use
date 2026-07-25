//===--- fn_def_args_tpl_redecl.h - iwyu test -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/fn_def_args_tpl_redecl-d1.h"

template <typename T>
// IWYU: FnTplDeclOnly(int, int, int) is...*fn_def_args_tpl_redecl-i1.h
typename T::Internal FnTplDeclOnly(int, int, int);

template <typename T>
// IWYU: FnTplNotCalled(int) is...*fn_def_args_tpl_redecl-i1.h
void FnTplNotCalled(int);

/**** IWYU_SUMMARY

tests/cxx/fn_def_args_tpl_redecl.h should add these lines:
#include "tests/cxx/fn_def_args_tpl_redecl-i1.h"

tests/cxx/fn_def_args_tpl_redecl.h should remove these lines:
- #include "tests/cxx/fn_def_args_tpl_redecl-d1.h"  // lines XX-XX

The full include-list for tests/cxx/fn_def_args_tpl_redecl.h:
#include "tests/cxx/fn_def_args_tpl_redecl-i1.h"  // for FnTplDeclOnly, FnTplNotCalled

***** IWYU_SUMMARY */
