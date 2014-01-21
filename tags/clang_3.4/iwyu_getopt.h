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

// Hand-rolled implementation of getopt/getopt_long for Visual C++.
extern const int no_argument;
extern const int required_argument;
extern const int optional_argument;

extern char* optarg;
extern int optind, opterr, optopt;

struct option {
  const char* name;
  int has_arg;
  int* flag;
  int val;
};

int getopt(int argc, char* const argv[], const char* optstring);

int getopt_long(int argc, char* const argv[],
  const char* optstring, const struct option* longopts, int* longindex);

#else  // #if defined(_MSC_VER)

#include <getopt.h>   // IWYU pragma: export

#endif  // #if defined(_MSC_VER)

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_GETOPT_H_
