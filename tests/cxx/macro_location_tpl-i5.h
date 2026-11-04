//===--- macro_location_tpl-i5.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

template <typename>
void spelling_template() = delete;
template <>
void spelling_template<int>();

template <typename>
struct SpellingTemplate {
  void method() {
  }
};
