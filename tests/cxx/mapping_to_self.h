//===--- mapping_to_self.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MAPPING_TO_SELF_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MAPPING_TO_SELF_H_

#include "tests/cxx/mapping_to_self-d1.h" // IWYU pragma: keep

// A mapping is defined from E to this file.  This test is to ensure that
// - That mapping is selected over another mapping.
// - The mapping does not force this file to add a #include of itself.
E e;

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MAPPING_TO_SELF_H_

/**** IWYU_SUMMARY

(tests/cxx/mapping_to_self.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
