//===--- specialization_use_count-provided.h - test input -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_count-inst.h"

// Provides the explicit instantiation of Box<Payload> to its users.
template <class T>
using P = ATQ<T, T::template Nested>;
