//===--- defn_is_use.h - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace ns1::ns2 {
class Class2InNs2;
}

extern ns1::ns2::Class2InNs2 c2ns2;

/**** IWYU_SUMMARY

(tests/cxx/defn_is_use.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
