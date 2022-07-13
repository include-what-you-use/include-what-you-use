//===--- indirect.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be #included by "direct.h", which is directly
// #included by some .cc file.  It provides an easy-to-access
// definition of a symbol that will cause an iwyu violation in a .cc
// file.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_INDIRECT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_INDIRECT_H_

class IndirectClass {
 public:
  void Method() const {
  }
  int a;
  static void StaticMethod() {
  }
  static int statica;
};

template <typename T>
class IndirectTemplate {
 public:
  void Method() const {
  }
  int a;
  static void StaticMethod() {
  }

 private:
  T t;
};


#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_INDIRECT_H_
