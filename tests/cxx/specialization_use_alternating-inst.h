//===--- specialization_use_alternating-inst.h - test input ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_alternating-defs.h"

extern template struct Box1<Payload>;
extern template struct Box2<Payload>;

struct Alpha {};
