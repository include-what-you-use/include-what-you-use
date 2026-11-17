//===--- configured_api.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A system header whose existing declarations change when the includer
// defines CONFIGURED_API_WIDE first, in the style of <picojson.h> and
// PICOJSON_USE_INT64.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CONFIGURED_API_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CONFIGURED_API_H_

class ConfiguredValue {
#ifdef CONFIGURED_API_WIDE
  long long wide;
#endif
  int narrow;
};

typedef ConfiguredValue* ConfiguredObject;

#endif
