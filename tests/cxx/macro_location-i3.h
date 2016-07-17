//===--- macro_location-i3.h - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define DECLARE_FRIEND(cls)  friend class cls

class Foo {};


// Reduced example from the Linux kernel:
// http://lxr.free-electrons.com/source/tools/include/linux/compiler.h#L112
//
// (The below test code is absolute nonsense, but it triggers the same IWYU
// behavior as the actual READ_ONCE macro.)
//
// This makes sure we don't treat anonymous type declarations as forward-decls
// in the context of macro use locations.
#define ANONYMOUS_TYPE_IN_MACRO(value) \
  char nonsense() {                    \
    union {                            \
      int v;                           \
      char c[1];                       \
    } u;                               \
    u.v = value;                       \
    return u.c[0];                     \
  }
