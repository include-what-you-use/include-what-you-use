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

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_INDIRECT_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_INDIRECT_H_

class IndirectClass {
 public:
  void Method() { };
  int a;
  static void StaticMethod() { };
  static int statica;
};


#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_INDIRECT_H_
