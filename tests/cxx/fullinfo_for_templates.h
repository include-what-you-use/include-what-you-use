//===--- fullinfo_for_templates.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// First include the file containing the definition of TemplateClass.
#include "tests/cxx/fullinfo_for_templates-d1.h"
// Then include a file containing a forward declaration of TemplateClass.
#include "tests/cxx/fullinfo_for_templates-d2.h"
// IWYU requires full info when typedefing a template.
typedef TemplateClass<int> tc_int;

/**** IWYU_SUMMARY

tests/cxx/fullinfo_for_templates.h should add these lines:

tests/cxx/fullinfo_for_templates.h should remove these lines:
- #include "tests/cxx/fullinfo_for_templates-d2.h"  // lines XX-XX

The full include-list for tests/cxx/fullinfo_for_templates.h:
#include "tests/cxx/fullinfo_for_templates-d1.h"  // for TemplateClass

***** IWYU_SUMMARY */
