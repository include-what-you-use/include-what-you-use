//===--- decltype-i1.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// No fields of type T so that the defaulted constructor doesn't need
// the complete parameter type.
template <typename T>
struct Tpl {
  static constexpr auto s = sizeof(T);
};
