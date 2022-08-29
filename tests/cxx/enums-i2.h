//===--- enums-i2.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_ENUMS_I2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_ENUMS_I2_H_

enum class IndirectEnum2 : int { A, B, C };

enum class IndirectEnum8 { A, B, C };

enum IndirectEnum9 : int { A9, B9, C9 };

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_ENUMS_I2_H_
