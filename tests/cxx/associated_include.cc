//===--- associated_include.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that if we remove an include from an 'associated' .h file, we
// add it to the .cc file, but if we keep an include in an
// 'associated' .h file, we don't try to add it to the .cc file.

#include "tests/cxx/associated_include.h"
#include "tests/cxx/associated_include-d1.h"
#include "tests/cxx/associated_include-d2.h"

IndirectClass ic;
AssociatedIncludeClass aic;

// IWYU should not consider "*-i2.h" as being present in the associated header
// (because it should be removed from that), hence the following two types
// should be attributed to "*-d1.h".
ClassExportedThroughD1 aic2;
ClassFromD1 aic3;

// Despite "*-d2.h" reexports ClassFromI3, IWYU should suggest to remove it
// because it should also suggest to add "*-i3.h" in the associated header.
// IWYU: ClassFromI3 is...*associated_include-i3.h
ClassFromI3 cfi3;

/**** IWYU_SUMMARY

tests/cxx/associated_include.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/associated_include.cc should remove these lines:
- #include "tests/cxx/associated_include-d2.h"  // lines XX-XX

The full include-list for tests/cxx/associated_include.cc:
#include "tests/cxx/associated_include.h"
#include "tests/cxx/associated_include-d1.h"  // for ClassExportedThroughD1, ClassFromD1
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
