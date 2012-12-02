//===--- badinc2.c - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is #included from badinc.cc.

extern "C" int C_Function(int x) { return 5; }
