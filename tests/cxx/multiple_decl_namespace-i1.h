//===--- multiple_decl_namespace-i1.h - test input file for IWYU ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_I1_H_

// Indirect includes of headers to force IWYU to complain
#include "tests/cxx/multiple_decl_namespace-d2.h"
#include "tests/cxx/multiple_decl_namespace-d3.h"
#include "tests/cxx/multiple_decl_namespace-d4.h"
#include "tests/cxx/multiple_decl_namespace-d5.h"
#include "tests/cxx/multiple_decl_namespace-d6.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MULTIPLE_DECL_NAMESPACE_I1_H_
