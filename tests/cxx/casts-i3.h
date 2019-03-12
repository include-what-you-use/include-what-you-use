//===--- casts-i3.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class CastsI2Derived;

struct CastsI3Convertible {
  operator CastsI2Derived*() {
    return nullptr;
  }
};
