//===--- implicit_ctor-i2.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct IndirectWithImplicitCtor {
  // This is the implicit ctor.
  IndirectWithImplicitCtor(int x) {}
};
