//===--- fn_def_args_tpl_redecl-i2.h - iwyu test --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T>
typename T::Internal FnTplDeclOnly(int, int, int);

template <typename T>
void FnTplNotCalled(int);
