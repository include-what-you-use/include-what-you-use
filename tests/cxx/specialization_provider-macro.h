//===--- specialization_provider-macro.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

// Spelling and expansion locations must not confuse which header provides.
#define DEFINE_QUALIFIER(name) \
  template <class T>           \
  using name = ArgumentThenQualifier<T, T::template Nested>;
