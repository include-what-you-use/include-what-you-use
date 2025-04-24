#pragma once

#include "table.h"  // IWYU pragma: export

template <typename T>
struct Map {
  Iterator<T> find() {
    Table<T> impl;
    return impl.find();
  }
};