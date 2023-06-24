//===--- iwyu_getopt.h - OS-specific implementation of getopt for IWYU ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_IWYU_GETOPT_H_
#define INCLUDE_WHAT_YOU_USE_IWYU_GETOPT_H_

#if defined(_MSC_VER)

#if defined(__cplusplus)
extern "C" {
#endif

#define no_argument 1
#define required_argument 2
#define optional_argument 3

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

#if defined(__cplusplus)
}
#endif

#else  // #if defined(_MSC_VER)

#include <getopt.h>   // IWYU pragma: export

#endif  // #if defined(_MSC_VER)

#endif  // INCLUDE_WHAT_YOU_USE_IWYU_GETOPT_H_
