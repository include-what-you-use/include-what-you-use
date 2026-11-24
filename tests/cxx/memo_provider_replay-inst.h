//===--- memo_provider_replay-inst.h - test input -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/memo_provider_replay-defs.h"
#include "tests/cxx/memo_provider_replay-payload.h"

extern template struct Box<Payload>;
