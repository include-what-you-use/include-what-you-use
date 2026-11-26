//===--- implementation_file_negative-i2.h - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_NEGATIVE_I2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_NEGATIVE_I2_H_

// Only compiles because -d1.h #includes indirect.h first.
inline int NegUsesIndirect(int i) {
  IndirectClass local;
  return local.a + i;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_NEGATIVE_I2_H_
