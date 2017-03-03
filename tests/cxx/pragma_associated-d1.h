//===--- pragma_associated-d1.h - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file does not provide any symbols, and only exists to be
// force-associated by `pragma associated`.
//
// Since it's associated, it will be analyzed, and needs the IWYU summary.

/**** IWYU_SUMMARY

(tests/cxx/pragma_associated-d1.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
