//===--- associated_include.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that if we remove an include from an 'associated' .h file, we
// add it to the .cc file, but if we keep an include in an
// 'associated' .h file, we don't try to add it to the .cc file.

#include "tests/cxx/associated_include.h"

IndirectClass ic;
AssociatedIncludeClass aic;


/**** IWYU_SUMMARY

tests/cxx/associated_include.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/associated_include.cc should remove these lines:

The full include-list for tests/cxx/associated_include.cc:
#include "tests/cxx/associated_include.h"
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
