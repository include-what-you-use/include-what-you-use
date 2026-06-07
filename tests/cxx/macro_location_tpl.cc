//===--- macro_location_tpl.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Expands on macro_location test case to also cover template scenarios.  Macro
// uses are attributed to spelling location, but forward-declarations of primary
// templates also work as author intent hints for template specializations used
// inside the macro.

// IWYU_ARGS: -I .

#include "tests/cxx/macro_location_tpl-d2.h"
#include "tests/cxx/macro_location_tpl-d1.h"
#include "tests/cxx/macro_location_tpl-d3.h"

DEFINE_CALL_FN3

void use_macro_with_template_spec() {
  // IWYU: FUNC_TEMPLATE_SPEC_EXPANSION is...*macro_location_tpl-i1.h
  FUNC_TEMPLATE_SPEC_EXPANSION;

  // IWYU: FUNC_TEMPLATE_SPEC_SPELLING is...*macro_location_tpl-i1.h
  FUNC_TEMPLATE_SPEC_SPELLING;

  // IWYU: CLASS_TEMPLATE_SPEC_EXPANSION is...*macro_location_tpl-i1.h
  CLASS_TEMPLATE_SPEC_EXPANSION;

  // IWYU: CLASS_TEMPLATE_SPEC_SPELLING is...*macro_location_tpl-i1.h
  CLASS_TEMPLATE_SPEC_SPELLING;

  // IWYU should report the macro-defined explicit specialization
  // of TplToSpecialize from '-i2.h' because it is not provided
  // by UseSpecializedTpl definition. This approximately resembles
  // Q_DECLARE_METATYPE(UserType) which should be available to be able to put
  // UserType into QVariant.
  // IWYU: UseSpecializedTpl() is...*macro_location_tpl-i1.h
  // IWYU: Struct needs a declaration
  // IWYU: TplToSpecialize<Struct>::EnumItem is...*macro_location_tpl-i2.h
  UseSpecializedTpl<Struct>();

  // Fn1 should not be reported here.
  // IWYU: Struct is...*macro_location_tpl-i1.h
  UseProvidedFn1Taking(Struct{});

  // IWYU should better not report Fn2 here: it is an internal stuff defined
  // along with TplFn during DEFINE_TPL_FN_AND_FN2 macro expansion.
  // IWYU: TplFn() is...*macro_location_tpl-i2.h
  // IWYU: Struct needs a declaration
  TplFn<Struct>();

  // Fn2 should not be reported here.
  // IWYU: Struct is...*macro_location_tpl-i1.h
  CallFn2WithProvidingMacro(Struct{});

  // Test that IWYU keeps '-d3.h' declaring Fn3.
  // IWYU: Struct is...*macro_location_tpl-i1.h
  CallFn3(Struct{});

  // DefArg1 is actually defined by macro expansion in '-i2.h' and is not
  // provided by the template function.
  // IWYU: UseNonProvidedDefArg1() is...*macro_location_tpl-i1.h
  // IWYU: DefArg1 is...*macro_location_tpl-i2.h
  UseNonProvidedDefArg1<>();

  // IWYU: UseProvidedDefArg2() is...*macro_location_tpl-i2.h
  UseProvidedDefArg2<>();

  UseProvidedDefArg3<>();
}

/**** IWYU_SUMMARY

tests/cxx/macro_location_tpl.cc should add these lines:
#include "tests/cxx/macro_location_tpl-i1.h"
#include "tests/cxx/macro_location_tpl-i2.h"

tests/cxx/macro_location_tpl.cc should remove these lines:

The full include-list for tests/cxx/macro_location_tpl.cc:
#include "tests/cxx/macro_location_tpl-d1.h"  // for CallFn2WithProvidingMacro, DEFINE_CALL_FN3, UseProvidedDefArg3, UseProvidedFn1Taking
#include "tests/cxx/macro_location_tpl-d2.h"  // for ExpansionTemplate, expansion_template
#include "tests/cxx/macro_location_tpl-d3.h"  // for Fn3
#include "tests/cxx/macro_location_tpl-i1.h"  // for CLASS_TEMPLATE_SPEC_EXPANSION, CLASS_TEMPLATE_SPEC_SPELLING, FUNC_TEMPLATE_SPEC_EXPANSION, FUNC_TEMPLATE_SPEC_SPELLING, Struct, UseNonProvidedDefArg1, UseSpecializedTpl
#include "tests/cxx/macro_location_tpl-i2.h"  // for DefArg1, TplFn, TplToSpecialize<>::EnumItem, UseProvidedDefArg2

***** IWYU_SUMMARY */
