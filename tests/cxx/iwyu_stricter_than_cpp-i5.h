//===--- iwyu_stricter_than_cpp-i5.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename T1, typename T2>
struct TplIndirectStruct3 {
  static constexpr auto s = sizeof(T1);
  T2* t2;
};
