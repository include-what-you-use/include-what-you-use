//===--- memo_provider_replay-nonprovided.h - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/memo_provider_replay-defs.h"
#include "tests/cxx/memo_provider_replay-macro.h"

template <class T>
using NonProvided = ArgumentThenQualifier<T, T::template Nested>;

DEFINE_QUALIFIER(NonProvidedByMacro)
