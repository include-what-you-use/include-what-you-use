//===--- implementation_file_negative-i5.h - test input file for iwyu -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_NEGATIVE_I5_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_NEGATIVE_I5_H_

// Defines a class of its own, and a function that -d1.h declares.
class NegNode {
 public:
  int value;
};

inline int NegTree::Size() const {
  return root->value;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_NEGATIVE_I5_H_
