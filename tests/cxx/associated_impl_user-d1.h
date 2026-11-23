//===--- associated_impl_user-d1.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

class UserInlineClass {
 public:
  void InlinedMethod();
};

#include "tests/cxx/associated_impl_user-d1.inline"
