//===--- associated_include_roundabout.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once

namespace hfile {
class RoundaboutClass { };
}

/**** IWYU_SUMMARY

(tests/cxx/associated_include_roundabout.h has correct #includes/fwd-decls)


***** IWYU_SUMMARY */
