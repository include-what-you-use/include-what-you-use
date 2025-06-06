//===--- include1.h - iwyu test -------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __INCLUDE1_H__

#ifndef __disabled_things
#define __INCLUDE1_H__ 1

struct importantthing {
  int x, y, z;
};
#define OPTION1 2
#define OPTION2 4

#endif

extern int superimportantvariable;  // always visible

#undef __disabled_things
#endif  //__INCLUDE1_H__
