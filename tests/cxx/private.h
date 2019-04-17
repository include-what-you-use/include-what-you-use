//===--- private.h - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be #included by "export_private_near.h".  The pragmas
// in these two headers mean that export_private.h is the normal header to
// include to access PrivateClass, rather than including this file directly.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PRIVATE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PRIVATE_H_

// IWYU pragma: private

class PrivateClass {
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PRIVATE_H_
