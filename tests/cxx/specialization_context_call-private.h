//===--- specialization_context_call-private.h - test input ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

struct Src {
  int x;
};

struct Conv {
  Conv(const Src&);
};

struct W1 {};
struct W2 {};
