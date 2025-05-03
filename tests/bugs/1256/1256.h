//===--- 1256.h - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

struct results;

struct funcs {
  // No diagnostic expected
  results (*get_results)();
};

/**** IWYU_SUMMARY

(tests/bugs/1256/1256.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
