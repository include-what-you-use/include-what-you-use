//===--- macro.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// macro.h
#define MACRO_A(domain) \
  void myfunc() { \
    static ns::Class s_var(ns::getmyclass##domain()); \
  }
