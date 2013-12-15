//===--- prefix_header_includes_add.cc - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests --prefix_header_includes option.  All prefix_header_includes_*.cc files
// are the same to show the difference between --prefix_header_includes values.

#include "tests/direct.h"
#include "tests/prefix_header_includes-d1.h"

// Included in source code and via command line option.
CommandLineIncludeD1 cli_d1;
// Included via command line option only.
// IWYU: CommandLineIncludeD2 is...*prefix_header_includes-d2.h
CommandLineIncludeD2 cli_d2;

// Forward declared in source code and included via command line option.
class CommandLineIncludeD3;
CommandLineIncludeD3* cli_d3_ptr;
// Included via command line option only.
// IWYU: CommandLineIncludeD4 needs a declaration
CommandLineIncludeD4* cli_d4_ptr;

// Test that is_prefix_header property is preserved for indirect includes.
// IWYU: CommandLineIncludeI1 is...*prefix_header_includes-i1.h
CommandLineIncludeI1 cli_i1;

// Test not prefix header include.
// IWYU: IndirectClass is...*indirect.h
IndirectClass ic;

/**** IWYU_SUMMARY

tests/prefix_header_includes_add.cc should add these lines:
#include "tests/indirect.h"
#include "tests/prefix_header_includes-d2.h"
#include "tests/prefix_header_includes-i1.h"
class CommandLineIncludeD4;

tests/prefix_header_includes_add.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/prefix_header_includes_add.cc:
#include "tests/indirect.h"  // for IndirectClass
#include "tests/prefix_header_includes-d1.h"  // for CommandLineIncludeD1
#include "tests/prefix_header_includes-d2.h"  // for CommandLineIncludeD2
#include "tests/prefix_header_includes-i1.h"  // for CommandLineIncludeI1
class CommandLineIncludeD3;  // lines XX-XX
class CommandLineIncludeD4;

***** IWYU_SUMMARY */
