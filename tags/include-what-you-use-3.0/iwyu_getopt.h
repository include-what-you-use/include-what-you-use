//===--- iwyu_getopt.h - OS-specific implementation of getopt for IWYU ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_GETOPT_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_GETOPT_H_

#if defined(_MSC_VER)

// We provide a partial implementation of getopt_long for Windows.
// For now the longopts struct is ignored, and only shortopts are parsed.
// This would all normally be defined in getopt.h.

struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

// Valid values for the 'has_arg' field of struct 'option'.
const int no_argument = 0;
const int required_argument = 1;
const int optional_argument = 2;

#else  // #if defined(_MSC_VER)

#include <getopt.h>   // IWYU pragma: export

#endif  // #if defined(_MSC_VER)

namespace include_what_you_use {

// Like getopt_long(), except that it outputs optarg and optind via
// output parameters instead of global variables.
int GetOptLong(int argc, char* const* argv, const char* shortopts,
               const option* longopts, const char** optarg, int* optind);
}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_GETOPT_H_
