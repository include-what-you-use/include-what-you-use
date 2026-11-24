//===--- specialization_use_count-defs.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <class... Ts>
struct Sink {
  template <class>
  struct Nested {};
};

template <class T>
struct Box {
  T value;
  template <class>
  struct Nested {};
};

template <class T, template <class> class N>
struct ATQ {
  template <class>
  struct Nested {};
};
