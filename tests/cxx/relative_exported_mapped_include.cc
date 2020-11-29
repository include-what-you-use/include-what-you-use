//===--- relative_exported_mapped_include.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu \
//            --mapping_file=tests/cxx/relative_exported_mapped_include.imp \
//            -I tests/cxx/subdir

// Ensure that when an include is added which is the public mapping of a
// symbol, that header can be added as a relative include rather than using a
// full path.

#include "relative_exported_mapped_include-d1.h"

MappedToExportedHeader x;

/**** IWYU_SUMMARY

(tests/cxx/relative_exported_mapped_include.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
