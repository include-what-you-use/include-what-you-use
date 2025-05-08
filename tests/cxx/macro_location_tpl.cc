//===--- macro_location_tpl.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Expands on macro_location test case to also cover template scenarios. All
// macro uses are attributed to expansion location.

// IWYU_ARGS: -I .

#include "tests/cxx/macro_location_tpl-d2.h"
#include "tests/cxx/macro_location_tpl-d1.h"

void use_macro_with_template_spec() {
  // IWYU: FUNC_TEMPLATE_SPEC_EXPANSION is defined...*macro_location_tpl-i1.h
  FUNC_TEMPLATE_SPEC_EXPANSION;

  // IWYU: FUNC_TEMPLATE_SPEC_SPELLING is defined...*macro_location_tpl-i1.h
  FUNC_TEMPLATE_SPEC_SPELLING;

  // IWYU: CLASS_TEMPLATE_SPEC_EXPANSION is defined...*macro_location_tpl-i1.h
  CLASS_TEMPLATE_SPEC_EXPANSION;

  // IWYU: CLASS_TEMPLATE_SPEC_SPELLING is defined...*macro_location_tpl-i1.h
  CLASS_TEMPLATE_SPEC_SPELLING;
}

/**** IWYU_SUMMARY

tests/cxx/macro_location_tpl.cc should add these lines:
#include "tests/cxx/macro_location_tpl-i1.h"

tests/cxx/macro_location_tpl.cc should remove these lines:
- #include "tests/cxx/macro_location_tpl-d1.h"  // lines XX-XX

The full include-list for tests/cxx/macro_location_tpl.cc:
#include "tests/cxx/macro_location_tpl-d2.h"  // for ExpansionTemplate, SpellingTemplate, expansion_template, spelling_template
#include "tests/cxx/macro_location_tpl-i1.h"  // for CLASS_TEMPLATE_SPEC_EXPANSION, CLASS_TEMPLATE_SPEC_SPELLING, FUNC_TEMPLATE_SPEC_EXPANSION, FUNC_TEMPLATE_SPEC_SPELLING

***** IWYU_SUMMARY */
