#pragma once

#include "map.h"

template <typename T>
bool Foo() {
  Map<T> map;
  Iterator<T> it = map.find();
  return it == it;
}
