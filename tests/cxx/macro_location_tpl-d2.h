//===--- macro_location_tpl-d2.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Declare a couple of function templates whose specializations are used in a
// macro, to check that author intent hints attribute uses to the right place.
template <typename>
void expansion_template() = delete;
template <>
void expansion_template<int>();

template <typename>
void spelling_template() = delete;
template <>
void spelling_template<int>();

// As above, but for class templates.
template <typename>
struct ExpansionTemplate {
  void method() {
  }
};

template <typename>
struct SpellingTemplate {
  void method() {
  }
};
