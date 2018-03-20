//===--- tolerate_transitive-direct.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TOLERATE_TRANSITIVE_DIRECT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TOLERATE_TRANSITIVE_DIRECT_H_

#include "tests/cxx/tolerate_transitive-indirect.h"

void DirectFunction(const IndirectType&);

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TOLERATE_TRANSITIVE_DIRECT_H_
