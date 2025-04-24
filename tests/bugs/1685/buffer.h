//===--- buffer.h - iwyu test ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

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
