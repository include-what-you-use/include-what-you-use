//===--- macro_location-i5.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file contains negative cases for the forward-declares used for
// use-attribution hints when expanding macros.

// An unnamed type inside a macro should not be seen by IWYU in files expanding
// the macro.
//
// The below test code is absolute nonsense, it's reduced from the definition of
// READ_ONCE in the Linux kernel:
// http://lxr.free-electrons.com/source/tools/include/linux/compiler.h#L112
#define UNNAMED_TYPE_IN_MACRO(value) \
  char nonsense() {                  \
    union {                          \
      int v;                         \
      char c[1];                     \
    } u;                             \
    u.v = value;                     \
    return u.c[0];                   \
  }
