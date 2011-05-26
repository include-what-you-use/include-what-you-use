//===--- iwyu_getopt.cc - OS-specific implementation of getopt for IWYU ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_getopt.h"

#include <string.h>

namespace include_what_you_use {

#if defined(_MSC_VER)

// This code is derived from http://www.codeproject.com/KB/cpp/xgetopt.aspx
// Originally written by Hans Dietrich, and released into the public domain.

int GetOptLong(int argc, char* const* argv, const char* shortopts,
               const option* longopts,
               const char** optarg_ptr, int* optind_ptr) {
  const char*& optarg = *optarg_ptr;
  int& optind = *optind_ptr;
  static char * next = NULL;
  if(optind == 0)
    next = NULL;
  optarg = NULL;
  if (next == NULL || *next == 0)
  {
    if (optind == 0)
      optind++;

    if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == 0)
    {
      optarg = NULL;
      if (optind < argc)
        optarg = argv[optind];
        return -1;
    }

    if (strcmp(argv[optind], "--") == 0)
    {
      optind++;
      optarg = NULL;
      if (optind < argc)
        optarg = argv[optind];
      return -1;
    }

    next = argv[optind];
    next++;    // skip past -
    optind++;
  }

  char c = *next++;
  const char *cp = strchr(shortopts, c);

  if (cp == NULL || c == ':')
    return '?';

  cp++;
  if (*cp == ':')
  {
    if (*next != 0)
    {
      optarg = next;
      next = NULL;
    }
    else if (optind < argc)
    {
      optarg = argv[optind];
      optind++;
    }
    else
    {
      return '?';
    }
  }

  return c;
}

#else  // #if defined(_MSC_VER)

int GetOptLong(int argc, char* const* argv, const char* shortopts,
               const option* longopts, const char** optarg, int* optind) {
  const int ret = getopt_long(argc, argv, shortopts, longopts, NULL);
  *optarg = ::optarg;
  *optind = ::optind;
  return ret;
}

#endif  // #if defined(_MSC_VER)

}  // namespace include_what_you_use
