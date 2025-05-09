//===--- include2.h - iwyu test -------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __INCLUDE2_H__
#define __INCLUDE2_H__

#define __disabled_things
#include "include1.h"
#undef __disabled_things

struct something2 {
  char text[32];
};
#endif  //__INCLUDE2_H__
