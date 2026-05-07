//===--- overload_fn_tpl-i3.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace ns {

// Two overloads in the same file.
template <typename T>
T TplFnSameFile(T);

int TplFnSameFile(int);

}  // namespace ns
