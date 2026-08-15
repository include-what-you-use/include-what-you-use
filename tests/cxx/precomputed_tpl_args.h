//===--- precomputed_tpl_args.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <set>

// Same as IntSet in .cc-file, but the typedef is handled as providing.
typedef std::set<int> HeaderIntSet;

/**** IWYU_SUMMARY

(tests/cxx/precomputed_tpl_args.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
