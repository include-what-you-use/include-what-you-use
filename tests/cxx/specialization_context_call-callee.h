//===--- specialization_context_call-callee.h - test input ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_context_call-private.h"

// Has a Conv parameter, which makes it provide Conv for autocasts in calls
// to it.
template <class A>
int Callee(A, Conv);
