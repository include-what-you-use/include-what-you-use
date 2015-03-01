//===--- fwd_decl_class_template.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that a class template is properly forward declared. i1.h has
// the following definition:
// template <typename T> class ClassTemplateI1 {};
// The .h file includes i1.h and uses ClassTemplateI1 in a
// forward-declarable way.
#include "tests/cxx/fwd_decl_class_template.h"
ClassTemplateI1<int>* cti1 = new ClassTemplateI1<int>();

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_class_template.cc should add these lines:
#include "tests/cxx/fwd_decl_class_template-i1.h"

tests/cxx/fwd_decl_class_template.cc should remove these lines:

The full include-list for tests/cxx/fwd_decl_class_template.cc:
#include "tests/cxx/fwd_decl_class_template.h"
#include "tests/cxx/fwd_decl_class_template-i1.h"  // for ClassTemplateI1

***** IWYU_SUMMARY */
