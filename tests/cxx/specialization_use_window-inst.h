//===--- specialization_use_window-inst.h - test input --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "tests/cxx/specialization_use_window-defs.h"
#include "tests/cxx/specialization_use_window-payload.h"

extern template struct Box<Payload>;
extern template struct Crate<Payload>;
