//===--- provided_sugar.cc - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also=tests/cxx/provided_sugar-provider.h

#include "tests/cxx/provided_sugar-provider.h"

void UseProvidedSugar() {
  ProvidedUsingReturn();
  ProvidedUsingArgument(1);
  ProvidedDecltypeReturn();
  ProvidedDecltypeArgument(2);
  ProvidedTemplateSugarReturn();
  ProvidedTypeofReturn();
  ProvidedTypeofArgument(3);
  ProvidedSameFileReturn();
}

/**** IWYU_SUMMARY

(tests/cxx/provided_sugar.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
