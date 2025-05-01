#pragma once

#include "memory.h"

template <typename T>
class Buffer {
  T* data_;

 public:
  ~Buffer() noexcept {
    destroy_memory(data_);
  }
};
