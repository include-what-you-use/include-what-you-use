//===--- specialization_provider-provided.h - test input ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_provider-inst.h"

template <class T>
using Provided = ArgumentThenQualifier<T, T::template Nested>;
