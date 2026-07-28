//===--- provided_sugar-provider.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/provided_sugar-decltype-defs.h"
#include "tests/cxx/provided_sugar-template-defs.h"
#include "tests/cxx/provided_sugar-template.h"
#include "tests/cxx/provided_sugar-typeof-defs.h"
#include "tests/cxx/provided_sugar-types.h"
#include "tests/cxx/provided_sugar-using-defs.h"
#include "tests/cxx/provided_sugar-using.h"

ns::UsingReturn ProvidedUsingReturn();
void ProvidedUsingArgument(ns::UsingArgument);
decltype(decltype_result) ProvidedDecltypeReturn();
void ProvidedDecltypeArgument(decltype(decltype_argument));
SugarTemplate<decltype(template_sugar_result)> ProvidedTemplateSugarReturn();

// TypeOfExprType verifies that provision is not limited to a fixed list of
// sugar kinds.
__typeof__(typeof_result) ProvidedTypeofReturn();
void ProvidedTypeofArgument(__typeof__(typeof_argument));

// A definition later in the same file provides the sugared return type without
// creating a redundant forward declaration here.
decltype(same_file_result) ProvidedSameFileReturn();

class SameFileReturn {};

/**** IWYU_SUMMARY

(tests/cxx/provided_sugar-provider.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
