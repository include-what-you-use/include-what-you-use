//===--- macro.h - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once
#include <string>

void UseString(const char* message);
std::string GetString();

// clang-format off
#define MY_MACRO() UseString(GetString() \
    .c_str())
// clang-format on
