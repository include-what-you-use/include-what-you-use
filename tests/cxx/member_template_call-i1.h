//===--- member_template_call-i1.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MEMBER_TEMPLATE_CALL_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MEMBER_TEMPLATE_CALL_I1_H_

class MemberTemplateClass {
 public:
  template <typename T>
  void Explicit() const {
  }
  template <typename T>
  void Deduced(T) const {
  }
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MEMBER_TEMPLATE_CALL_I1_H_
