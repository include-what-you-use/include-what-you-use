//===--- macro_defined_by_includer_system-d1.h - test input file for iwyu -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Opts in to the extended API of one system header, and configures
// another, before including them.

#define OPT_IN_API_EXTENDED
#include <opt_in_api.h>

#define CONFIGURED_API_WIDE
#include <configured_api.h>

struct ConfiguredUser {
  OptInExtended extended;
};
