//===--- associated_impl_discovery.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/associated_impl_discovery.imp -I .

// Tests that the implementation file of the main file's own header is
// analyzed without '--check_also', as that header itself is.

#include "tests/cxx/associated_impl_discovery.h"

void Use() {
  DiscoveryInlineClass c;
}

/**** IWYU_SUMMARY

(tests/cxx/associated_impl_discovery.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
