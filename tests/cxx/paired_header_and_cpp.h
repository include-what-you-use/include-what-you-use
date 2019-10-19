//===- paired_header_and_cpp.h -- test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

class PairedHeaderAndCpp {
 public:
  PairedHeaderAndCpp();
};

/**** IWYU_SUMMARY
(tests/cxx/paired_header_and_cpp.h has correct #includes/fwd-decls)
***** IWYU_SUMMARY */
