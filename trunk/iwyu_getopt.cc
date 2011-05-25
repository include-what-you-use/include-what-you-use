//===--- iwyu_getopt.cc - OS specific implementation of getopt for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_getopt.h"

#if defined(_MSC_VER)

#include <string.h>

// This code is derived from http://www.codeproject.com/KB/cpp/xgetopt.aspx
// Originally written by Hans Dietrich, and released into the public domain.

char *optarg = NULL;
int optind = 0;

int getopt_long(int argc, char *const *argv, const char *shortopts,
                const struct option *longopts, int *longind)
{
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

#endif  // #if defined(_MSC_VER)
