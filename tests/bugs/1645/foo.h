//===--- foo.h - iwyu test ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "foo_private.h"  // IWYU pragma: export

int foo();
int foo(foo_t);
