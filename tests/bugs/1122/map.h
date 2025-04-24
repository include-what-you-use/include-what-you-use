//===--- map.h - iwyu test ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "table.h"  // IWYU pragma: export

template <typename T>
struct Map {
  Iterator<T> find() {
    Table<T> impl;
    return impl.find();
  }
};
