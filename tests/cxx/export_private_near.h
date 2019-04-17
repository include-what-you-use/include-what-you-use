//===--- export_private_near.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be a generic exporting header.  Similarly to
// export_near.h, it exports another header.  However, unlike that file, this
// one exports private.h, which is marked private, and therefore IWYU should
// not allow you to directly include private.h.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_PRIVATE_NEAR_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_PRIVATE_NEAR_H_

#include "private.h" // IWYU pragma: export

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_EXPORT_PRIVATE_NEAR_H_
