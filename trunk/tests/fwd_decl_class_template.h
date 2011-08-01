//===--- fwd_decl_class_template.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_FWD_DECL_CLASS_TEMPLATE_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_FWD_DECL_CLASS_TEMPLATE_H_

#include "tests/fwd_decl_class_template-i1.h"

// TODO(dsturtevant): No iwyu violation is reported here, since there
// is none (the definition is in included file). However, iwyu
// recommends the inclusion be replaced by a forward declaration. It
// seems like some sort of diagnostic to that effect would be useful.
extern ClassTemplateI1<int>* cti1;

/**** IWYU_SUMMARY

tests/fwd_decl_class_template.h should add these lines:
template <typename T> class ClassTemplateI1;

tests/fwd_decl_class_template.h should remove these lines:
- #include "tests/fwd_decl_class_template-i1.h"  // lines XX-XX

The full include-list for tests/fwd_decl_class_template.h:
template <typename T> class ClassTemplateI1;

***** IWYU_SUMMARY */

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_FWD_DECL_CLASS_TEMPLATE_H_
