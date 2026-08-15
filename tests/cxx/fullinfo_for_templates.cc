//===--- fullinfo_for_templates.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test that when a template is typedef'd that the location of the definition, not
// any forward declaration, is included.

#include "tests/cxx/fullinfo_for_templates.h"
// First include the file containing the definition of TemplateClass.
#include "tests/cxx/fullinfo_for_templates-d1.h"
// Then include a file containing a forward declaration of TemplateClass.
#include "tests/cxx/fullinfo_for_templates-d2.h"
// A forward-declaration is sufficient here because the typedef is in a source
// file. See fullinfo_for_templates.h for the test case with a providing
// typedef.
typedef TemplateClass<int> tc_int;

/**** IWYU_SUMMARY

tests/cxx/fullinfo_for_templates.cc should add these lines:

tests/cxx/fullinfo_for_templates.cc should remove these lines:
- #include "tests/cxx/fullinfo_for_templates-d2.h"  // lines XX-XX

The full include-list for tests/cxx/fullinfo_for_templates.cc:
#include "tests/cxx/fullinfo_for_templates.h"
#include "tests/cxx/fullinfo_for_templates-d1.h"  // for TemplateClass (ptr only)

***** IWYU_SUMMARY */
