//===--- clmode.cc - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test will be executed with --driver-mode=cl and some MSVC-shaped
// flags to ensure we can run IWYU with MSVC-compatible command-line switches.

#include "tests/cxx/direct.h"

// This use isn't really important, we just want to make sure IWYU does
// something reasonable even in CL driver mode.
// IWYU: IndirectClass is...*indirect.h
IndirectClass random_use;

/**** IWYU_SUMMARY

tests/cxx/clmode.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/clmode.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/clmode.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
