//===--- macro_location_tpl-d2.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename>
void expansion_template() = delete;
template <>
void expansion_template<int>();

template <typename>
struct ExpansionTemplate {
  void method() {
  }
};
