//===--- specialization_provider-nonprovided.h - test input ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_provider-defs.h"

template <class T>
using NonProvided = ArgumentThenQualifier<T, T::template Nested>;
