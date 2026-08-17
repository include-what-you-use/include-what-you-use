//===--- provided_sugar_nonprovider.cc - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . \
//   -Xiwyu --check_also=tests/cxx/provided_sugar_nonprovider-api.h

#include "tests/cxx/provided_sugar-decltype-defs.h"
#include "tests/cxx/provided_sugar_nonprovider-api.h"
#include "tests/cxx/provided_sugar-typeof-defs.h"
#include "tests/cxx/provided_sugar-using-defs.h"

void UseNonProvidingSugar() {
  NonProvidingUsingReturn();
  NonProvidingUsingArgument(1);
  NonProvidingDecltypeReturn();
  NonProvidingDecltypeArgument(2);
  // IWYU: NestedSugarReturn is...*provided_sugar-nested-defs.h
  NonProvidingNestedSugarReturn();
  // IWYU: TemplateSugarReturn is...*provided_sugar-template-defs.h
  NonProvidingTemplateSugarReturn();
  NonProvidingTypeofReturn();
  NonProvidingTypeofArgument(3);
}

/**** IWYU_SUMMARY

tests/cxx/provided_sugar_nonprovider.cc should add these lines:
#include "tests/cxx/provided_sugar-nested-defs.h"
#include "tests/cxx/provided_sugar-template-defs.h"

tests/cxx/provided_sugar_nonprovider.cc should remove these lines:

The full include-list for tests/cxx/provided_sugar_nonprovider.cc:
#include "tests/cxx/provided_sugar-decltype-defs.h"  // for DecltypeArgument, DecltypeReturn
#include "tests/cxx/provided_sugar-nested-defs.h"  // for NestedSugarReturn
#include "tests/cxx/provided_sugar-template-defs.h"  // for TemplateSugarReturn
#include "tests/cxx/provided_sugar-typeof-defs.h"  // for TypeofArgument, TypeofReturn
#include "tests/cxx/provided_sugar-using-defs.h"  // for UsingArgument, UsingReturn
#include "tests/cxx/provided_sugar_nonprovider-api.h"  // for NonProvidingDecltypeArgument, NonProvidingDecltypeReturn, NonProvidingNestedSugarReturn, NonProvidingTemplateSugarReturn, NonProvidingTypeofArgument, NonProvidingTypeofReturn, NonProvidingUsingArgument, NonProvidingUsingReturn

***** IWYU_SUMMARY */
