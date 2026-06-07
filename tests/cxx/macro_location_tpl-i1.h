//===--- macro_location_tpl-i1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_TPL_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_TPL_I1_H_

// Forward-declare the primary templates as a signal that specialization uses
// belong in the expansion location.
template <typename>
void expansion_template();

template <typename>
class ExpansionTemplate;

// expansion_template should be attributed to expansion loc.
#define FUNC_TEMPLATE_SPEC_EXPANSION expansion_template<int>()

// spelling_template should be attributed to this file, because there's no
// forward-declare hint.
#define FUNC_TEMPLATE_SPEC_SPELLING spelling_template<int>();

// ExpansionTemplate should be attributed to expansion loc.
#define CLASS_TEMPLATE_SPEC_EXPANSION ExpansionTemplate<int>().method();

// SpellingTemplate should be attributed to this file, because there's no
// forward-declare hint.
#define CLASS_TEMPLATE_SPEC_SPELLING SpellingTemplate<int>().method();

template <typename>
struct TplToSpecialize {};

#define SPECIALIZE_TPL(Type)     \
  template <>                    \
  struct TplToSpecialize<Type> { \
    enum { EnumItem };           \
  };

template <typename T>
void UseSpecializedTpl() {
  (void)TplToSpecialize<T>::EnumItem;
}

struct Struct {};

#define DEFINE_DEF_ARG_1 \
  class DefArg1 {};

class DefArg1;

template <typename T = DefArg1>
void UseNonProvidedDefArg1() {
  T t;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_LOCATION_TPL_I1_H_
