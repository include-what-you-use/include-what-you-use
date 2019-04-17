//===--- relative_include_of_export_added.cc - test input file for iwyu ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// The purpose of this test is to ensure that when an include is added which is
// the public counterpart to a private header, that header can be added as a
// relative include rather than using a full path.
#include "relative_include_of_export_added-d1.h"

// IWYU: PrivateClass is...*"export_private_near.h"
PrivateClass x;

/**** IWYU_SUMMARY

tests/cxx/relative_include_of_export_added.cc should add these lines:
#include "export_private_near.h"

tests/cxx/relative_include_of_export_added.cc should remove these lines:
- #include "relative_include_of_export_added-d1.h"  // lines XX-XX

The full include-list for tests/cxx/relative_include_of_export_added.cc:
#include "export_private_near.h"  // for PrivateClass

***** IWYU_SUMMARY */
