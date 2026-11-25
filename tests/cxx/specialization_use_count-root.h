//===--- specialization_use_count-root.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_provider-provided.h"
#include "tests/cxx/specialization_provider-nonprovided.h"

template <class T>
using CountBoth = Sink<Provided<T>, NonProvided<T>>;

// Redundant alias paths must not amplify the explicit-instantiation use.
template <class T>
struct Holder {
  CountBoth<CountBoth<CountBoth<Box<T>>>> value;
};
