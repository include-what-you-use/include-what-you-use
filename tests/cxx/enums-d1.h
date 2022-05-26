//===--- enums-d1.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/enums-i1.h"
#include "tests/cxx/enums-i2.h"
#include "tests/cxx/enums-i3.h"
#include "tests/cxx/enums-i4.h"

enum class DirectEnum1 { A, B, C };

enum class DirectEnum2 : int { A, B, C };

enum struct DirectEnum3 : unsigned long long { A, B, C };

namespace ns {
enum DirectEnum4 : int { A, B, C };
}

enum class DirectEnum5 : long { A, B, C };
