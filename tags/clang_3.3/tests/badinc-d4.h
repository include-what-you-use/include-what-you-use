//===--- badinc-d4.h - test input file for iwyu ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file tests operators and references.  Even though
// badinc.cc only uses D4_ClassForOperator as a reference,
// we still need to #include this file to get operator<<.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D4_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D4_H_

class D4_ClassForOperator {
 public:
  int a() { return 1; }
};

int operator<<(int i, const D4_ClassForOperator& d4) {
  return i;
}

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_BADINC_D4_H_
