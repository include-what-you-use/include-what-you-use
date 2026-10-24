//===--- associated_mapping_cycle.h - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_MAPPING_CYCLE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_MAPPING_CYCLE_H_

class CycleClass {};

#include "tests/cxx/associated_mapping_cycle.icc"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_MAPPING_CYCLE_H_

/**** IWYU_SUMMARY

(tests/cxx/associated_mapping_cycle.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
