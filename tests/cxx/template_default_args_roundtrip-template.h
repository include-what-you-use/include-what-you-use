//===--- template_default_args_roundtrip-template.h - test input ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TEMPLATE_DEFAULT_ARGS_ROUNDTRIP_TEMPLATE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TEMPLATE_DEFAULT_ARGS_ROUNDTRIP_TEMPLATE_H_

template <class T>
struct Type {
  struct Base {
    T t;
  };
  struct NestedType : Base {};
};

template <class T>
struct DefaultArgument;

template <class T, class U = DefaultArgument<T>>
struct Template {
  typename Type<U>::NestedType t;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TEMPLATE_DEFAULT_ARGS_ROUNDTRIP_TEMPLATE_H_
