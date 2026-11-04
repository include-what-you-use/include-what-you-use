//===--- instantiated_template_specialization_cache.cc - test input -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/direct.h"
#include "tests/cxx/instantiated_template_specialization_cache-direct.h"

// Nested aliases reach the same specialization through independent paths.
// Indirect includes make missing uses visible in the diagnostics.
// IWYU: Holder is...*instantiated_template_specialization_cache-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Holder<IndirectClass> instance;

// A separate caller must retain its uses even after the first scan.
void AnotherCaller() {
  // IWYU: Holder is...*instantiated_template_specialization_cache-indirect.h
  // IWYU: IndirectClass is...*indirect.h
  // IWYU: IndirectClass needs a declaration
  Holder<IndirectClass> local;
  (void)local;
}

/**** IWYU_SUMMARY

tests/cxx/instantiated_template_specialization_cache.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/instantiated_template_specialization_cache-indirect.h"

tests/cxx/instantiated_template_specialization_cache.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/instantiated_template_specialization_cache-direct.h"  // lines XX-XX

The full include-list for tests/cxx/instantiated_template_specialization_cache.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/instantiated_template_specialization_cache-indirect.h"  // for Holder

***** IWYU_SUMMARY */
