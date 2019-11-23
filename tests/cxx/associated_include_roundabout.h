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
class AssociatedIncludeClass { };
}

/**** IWYU_SUMMARY

tests/cxx/associated_include_roundabout.h should add these lines:

tests/cxx/associated_include_roundabout.h should remove these lines:

The full include-list for tests/cxx/associated_include_roundabout.h:

***** IWYU_SUMMARY */
