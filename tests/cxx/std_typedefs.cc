//===--- std_typedefs.cc - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Wno-user-defined-literals

// Tests handling typedefs provided by more than one standard library header.
// Because wspanstream is used in the context requiring complete type info, it
// should be attributed to <spanstream> and not to <iosfwd>.

#include "tests/cxx/std_symbol_mapping-direct.h"

// IWYU: std::wspanstream is...*<spanstream>
// IWYU: std::basic_spanstream is...*<spanstream>
auto s = sizeof(std::wspanstream);

/**** IWYU_SUMMARY

tests/cxx/std_typedefs.cc should add these lines:
#include <spanstream>

tests/cxx/std_typedefs.cc should remove these lines:
- #include "tests/cxx/std_symbol_mapping-direct.h"  // lines XX-XX

The full include-list for tests/cxx/std_typedefs.cc:
#include <spanstream>  // for basic_spanstream, wspanstream

***** IWYU_SUMMARY */
