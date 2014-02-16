//===--- no_fwd_decl_nested_class.cc - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we use a nested class, and we already have an
// #include for the outer class, we don't try to forward-declare the
// nested class anyway.  Usually we got this right, but sometimes
// wouldn't when then nested class had no definition.

#include "tests/cxx/no_fwd_decl_nested_class-d1.h"

DirectOuterClass doc;
void CallDOC(DirectOuterClass::NestedClass*) { }

/**** IWYU_SUMMARY

(tests/cxx/no_fwd_decl_nested_class.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
