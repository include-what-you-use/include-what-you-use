//===--- macro_location_tpl-i4.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_TPL_I4_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_TPL_I4_H_

struct Struct;

void Fn2(const Struct&);

template <typename>
void spelling_template() = delete;
template <>
void spelling_template<int>();

template <typename>
struct SpellingTemplate {
  void method() {
  }
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_TPL_I4_H_
