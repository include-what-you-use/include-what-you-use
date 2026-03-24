//===--- print.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>

template <typename T>
void print(T value) {
    std::cout << "Generic value: " << value << '\n';
}

template <>
inline void print(bool value) {
    std::cout << "Boolean value: " << (value ? "TRUE" : "FALSE") << '\n';
}
