//===--- type_pack_elem.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"

// IWYU: IndirectClass is...*indirect.h
using Providing = IndirectClass;

/**** IWYU_SUMMARY

tests/cxx/type_pack_elem.h should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/type_pack_elem.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/type_pack_elem.h:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
