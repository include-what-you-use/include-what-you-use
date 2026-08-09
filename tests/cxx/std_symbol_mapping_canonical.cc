//===--- std_symbol_mapping_canonical.cc - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Wno-user-defined-literals

// Tests that symbols from std are by default attributed to their canonical
// headers (<iosfwd> for std::fstream in a forward-declarable context). For this
// test to work correctly, std::fstream should be the only reason to include
// <iosfwd> here.

#include "tests/cxx/std_symbol_mapping-direct.h"

// IWYU: std::fstream is...*<iosfwd>
std::fstream* fs;

// <tuple> is the canonical source of std::tuple declaration although <utility>
// should provide a forward-declaration too.
// IWYU: std::tuple needs a declaration
std::tuple<int, char, double>* t;

// The same for 'std::tm': <ctime> is its canonical header and not <cwchar>.
// IWYU: std::tm needs a declaration
std::tm* tm1;

/**** IWYU_SUMMARY

tests/cxx/std_symbol_mapping_canonical.cc should add these lines:
#include <ctime>
#include <iosfwd>
#include <tuple>

tests/cxx/std_symbol_mapping_canonical.cc should remove these lines:
- #include "tests/cxx/std_symbol_mapping-direct.h"  // lines XX-XX

The full include-list for tests/cxx/std_symbol_mapping_canonical.cc:
#include <ctime>  // for tm (ptr only)
#include <iosfwd>  // for fstream
#include <tuple>  // for tuple (ptr only)

***** IWYU_SUMMARY */
