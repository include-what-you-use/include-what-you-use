//===--- specialization_use_count-root.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_count-provided.h"
#include "tests/cxx/specialization_use_count-nonprovided.h"

template <class T>
using Both = Sink<P<T>, NP<T>>;

// Reaches Box<Payload> as a qualifier through NP once for every path through
// the nested aliases, which IWYU reports as a use of its explicit
// instantiation each time.
template <class T>
struct Holder {
  Both<Both<Both<Box<T>>>> value;
};
