//===--- specialization_qualifier-indirect.h - test input -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SPECIALIZATION_QUALIFIER_INDIRECT_H_
#define SPECIALIZATION_QUALIFIER_INDIRECT_H_

template <class T>
struct Box {
  T value;
  template <class U> struct Nested {};
};

template <class T, template <class> class Template>
struct ArgumentThenQualifier {
  T* pointer;
  Template<int> value;
};

template <class T>
using ForwardFirst = ArgumentThenQualifier<T, T::template Nested>;

template <class T>
struct Holder {
  ForwardFirst<Box<T>> value;
};

#endif  // SPECIALIZATION_QUALIFIER_INDIRECT_H_
