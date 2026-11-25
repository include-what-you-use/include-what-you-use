//===--- specialization_provider-defs.h - test input ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <class...> struct Sink {};

template <class T> struct Box {
  T value;
  template <class> struct Nested {};
};

template <class T, template <class> class N>
struct ArgumentThenQualifier {};
