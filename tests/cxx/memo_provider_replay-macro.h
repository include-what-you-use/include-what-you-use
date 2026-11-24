//===--- memo_provider_replay-macro.h - test input ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

// The aliases are spelled here, but expanded in the headers which do or
// don't provide the explicit instantiation.
#define DEFINE_QUALIFIER(name) \
  template <class T>           \
  using name = ArgumentThenQualifier<T, T::template Nested>;
