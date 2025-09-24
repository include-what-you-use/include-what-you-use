//===--- type_trait_spaceship.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++20

#include "tests/cxx/type_trait_spaceship-d1.h"

static_assert(__builtin_le_synthesises_from_spaceship(const Foo&, const Bar&));
