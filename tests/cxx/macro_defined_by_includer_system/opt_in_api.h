//===--- opt_in_api.h - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// A system header whose extended API is only declared when the includer
// defines OPT_IN_API_EXTENDED first, in the style of <lz4frame.h> and
// LZ4F_STATIC_LINKING_ONLY.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_OPT_IN_API_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_OPT_IN_API_H_

struct OptInBase {};

#endif

#if defined(OPT_IN_API_EXTENDED) && \
    !defined(INCLUDE_WHAT_YOU_USE_TESTS_CXX_OPT_IN_API_EXTENDED_H_)
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_OPT_IN_API_EXTENDED_H_

struct OptInExtended {};

#endif
