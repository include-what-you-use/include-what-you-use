//===--- fwd_decl_mapping-d1.h - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/fwd_decl_mapping-i1.h"

// Test that IWYU doesn't suggest that this header should include itself.
// TODO(bolshakov): wouldn't be better to suggest a forward-declaration here
// despite of the mapping? Is this case related to real life at all?
extern MappedToD1 mtd1;

/**** IWYU_SUMMARY

(tests/cxx/fwd_decl_mapping-d1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
