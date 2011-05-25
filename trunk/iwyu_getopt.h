//===--- iwyu_getopt.h - OS specific implementation of getopt for iwyu ----===//
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
extern char *optarg;
extern int optind;

#define no_argument             0
#define required_argument       1
#define optional_argument       2

struct option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

extern int getopt_long(int argc, char *const *argv, const char *shortopts,
                       const struct option *longopts, int *longind);


#else  // #if defined(_MSC_VER)

#include <getopt.h>   // IWYU pragma: export

#endif  // #if defined(_MSC_VER)

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_GETOPT_H_
