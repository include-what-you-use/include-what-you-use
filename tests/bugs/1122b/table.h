//===--- table.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

template <typename T>
struct Iterator {};

template <typename T>
bool operator==(const Iterator<T>& lhs, const Iterator<T>& rhs) {
  return true;
}

template <typename T>
struct Table {
  Iterator<T> find() { return Iterator<T>(); }
};
