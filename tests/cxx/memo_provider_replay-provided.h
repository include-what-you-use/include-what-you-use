//===--- memo_provider_replay-provided.h - test input ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/memo_provider_replay-inst.h"
#include "tests/cxx/memo_provider_replay-macro.h"

// Uses its argument as a qualifier, which needs the full type.
template <class T>
using Provided = ArgumentThenQualifier<T, T::template Nested>;

DEFINE_QUALIFIER(ProvidedByMacro)
