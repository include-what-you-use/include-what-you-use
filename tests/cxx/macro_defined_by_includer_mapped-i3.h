//===--- macro_defined_by_includer_mapped-i3.h - test input file for iwyu -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Private to -d3.h by a mapping file.  Like glibc <bits/*.h>, it refuses to
// be included other than through its public header.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_MAPPED_D3_H_
#error "Include macro_defined_by_includer_mapped-d3.h instead."
#endif

struct MappedPrivateBase {};

#ifdef MAPPED_PRIVATE_FEATURE
struct MappedPrivateFeature {};
#endif
