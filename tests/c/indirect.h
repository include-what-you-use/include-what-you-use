//===--- indirect.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be #included by "direct.h", which is directly
// #included by some .c file.  It provides an easy-to-access
// definition of a symbol that will cause an iwyu violation in a .c
// file.

struct Indirect {
    int a;
};
