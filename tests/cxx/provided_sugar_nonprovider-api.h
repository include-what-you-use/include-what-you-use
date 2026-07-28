//===--- provided_sugar_nonprovider-api.h - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/provided_sugar-transitive.h"
#include "tests/cxx/provided_sugar-template.h"

// IWYU: UsingReturn needs a declaration
// IWYU: ns::UsingReturn is...*provided_sugar-using.h...*for using decl
ns::UsingReturn NonProvidingUsingReturn();
// IWYU: UsingArgument needs a declaration
// IWYU: ns::UsingArgument is...*provided_sugar-using.h...*for using decl
void NonProvidingUsingArgument(ns::UsingArgument);
// IWYU: decltype_result is...*provided_sugar-types.h
decltype(decltype_result) NonProvidingDecltypeReturn();
// IWYU: decltype_argument is...*provided_sugar-types.h
void NonProvidingDecltypeArgument(decltype(decltype_argument));
// The outer transform does not expose the type concealed by decltype.
// IWYU: nested_sugar_result is...*provided_sugar-types.h
__remove_pointer(decltype(nested_sugar_result)) NonProvidingNestedSugarReturn();
// IWYU: template_sugar_result is...*provided_sugar-types.h
SugarTemplate<decltype(template_sugar_result)>
NonProvidingTemplateSugarReturn();
// IWYU: typeof_result is...*provided_sugar-types.h
__typeof__(typeof_result) NonProvidingTypeofReturn();
// IWYU: typeof_argument is...*provided_sugar-types.h
void NonProvidingTypeofArgument(__typeof__(typeof_argument));

/**** IWYU_SUMMARY

tests/cxx/provided_sugar_nonprovider-api.h should add these lines:
#include "tests/cxx/provided_sugar-types.h"
#include "tests/cxx/provided_sugar-using.h"
class UsingArgument;
class UsingReturn;

tests/cxx/provided_sugar_nonprovider-api.h should remove these lines:
- #include "tests/cxx/provided_sugar-transitive.h"  // lines XX-XX

The full include-list for tests/cxx/provided_sugar_nonprovider-api.h:
#include "tests/cxx/provided_sugar-template.h"  // for SugarTemplate
#include "tests/cxx/provided_sugar-types.h"  // for decltype_argument, decltype_result, nested_sugar_result, template_sugar_result, typeof_argument, typeof_result
#include "tests/cxx/provided_sugar-using.h"  // for UsingArgument, UsingReturn
class UsingArgument;
class UsingReturn;

***** IWYU_SUMMARY */
