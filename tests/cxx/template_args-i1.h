//===--- template_args-i1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T>
struct TplInI1 {
  T t;
};

struct TplHost {
  template <typename>
  struct InnerTpl {};
};
