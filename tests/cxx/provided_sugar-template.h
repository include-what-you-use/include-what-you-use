//===--- provided_sugar-template.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_TEMPLATE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_TEMPLATE_H_

template <typename T>
class SugarTemplate {
  T value_;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_TEMPLATE_H_
