//===--- specialization_use_count-nonprovided.h - test input --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_count-defs.h"

template <class T>
using NP = ATQ<T, T::template Nested>;
