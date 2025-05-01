#pragma once
#include <string>

void UseString(const char* message);
std::string GetString();

// clang-format off
#define MY_MACRO() UseString(GetString() \
    .c_str())
// clang-format on
