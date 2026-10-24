//===--- specialization_context-indirect.h - test input -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SPECIALIZATION_CONTEXT_INDIRECT_H_
#define SPECIALIZATION_CONTEXT_INDIRECT_H_

template <class T>
struct Box {
  T value;
};

template <class T>
using Alias = Box<T>;

// Both orders must propagate the full use of T to the caller.
template <class T>
struct PointerThenValue {
  Alias<T>* pointer;
  Alias<T> value;
};

template <class T>
struct ValueThenPointer {
  Alias<T> value;
  Alias<T>* pointer;
};

template <class T>
struct DirectPointerThenValue {
  Box<T>* pointer;
  Box<T> value;
};

template <class T>
struct DirectValueThenPointer {
  Box<T> value;
  Box<T>* pointer;
};

template <class Left, class Right>
struct Pair {
  Left left;
  Right right;
};

template <class T>
using SharedPointerThenValue = Pair<T*, T>;

template <class T>
using SharedValueThenPointer = Pair<T, T*>;

template <class T>
struct SharedPointerFirst {
  SharedPointerThenValue<Box<T>> value;
};

template <class T>
struct SharedValueFirst {
  SharedValueThenPointer<Box<T>> value;
};

#endif  // SPECIALIZATION_CONTEXT_INDIRECT_H_
