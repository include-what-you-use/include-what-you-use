//===--- fwd_decl_mapping.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --mapping_file=tests/cxx/fwd_decl_mapping.imp \
//            -Xiwyu --check_also=tests/cxx/fwd_decl_mapping-d1.h

// Tests symbol mappings for forward-declarations.

#include "tests/cxx/fwd_decl_mapping-d1.h"

// IWYU: OnlyFwdDeclUse needs a declaration
OnlyFwdDeclUse* pOnlyFwdDeclUse;

// The forward-declaration header ('-i3.h') is not needed because the definition
// should be available.
// IWYU: FwdDeclAndFullUse needs a declaration
FwdDeclAndFullUse* pFwdDeclAndFullUse;
// IWYU: FwdDeclAndFullUse is...*-i4.h
FwdDeclAndFullUse fwdDeclAndFullUse;

// '-i5.h' should not be suggested because '-i6.h' should provide them both.
// IWYU: FwdDeclInTwoHeaders1 needs a declaration
FwdDeclInTwoHeaders1* fdth1;
// IWYU: FwdDeclInI6 needs a declaration
FwdDeclInI6* fdi6;

// IWYU: HasIncludeMapping needs a declaration
HasIncludeMapping* him;

// IWYU: HasMappingForFullUse needs a declaration
HasMappingForFullUse* hmffu;

// Test that IWYU suggests the canonical (first mapped) header.
// IWYU: FwdDeclInTwoHeaders2 needs a declaration
FwdDeclInTwoHeaders2* fdth2;

// IWYU: EnumClass1 needs a declaration
EnumClass1 e1;

// IWYU: EnumClass2 needs a declaration
// IWYU: EnumClass2 is...*-i14.h
EnumClass2 e2 = EnumClass2::A;

/**** IWYU_SUMMARY

tests/cxx/fwd_decl_mapping.cc should add these lines:
#include "tests/cxx/fwd_decl_mapping-i10.h"
#include "tests/cxx/fwd_decl_mapping-i12.h"
#include "tests/cxx/fwd_decl_mapping-i14.h"
#include "tests/cxx/fwd_decl_mapping-i2.h"
#include "tests/cxx/fwd_decl_mapping-i4.h"
#include "tests/cxx/fwd_decl_mapping-i6.h"
#include "tests/cxx/fwd_decl_mapping-i9.h"

tests/cxx/fwd_decl_mapping.cc should remove these lines:
- #include "tests/cxx/fwd_decl_mapping-d1.h"  // lines XX-XX

The full include-list for tests/cxx/fwd_decl_mapping.cc:
#include "tests/cxx/fwd_decl_mapping-i10.h"  // for FwdDeclInTwoHeaders2 (ptr only)
#include "tests/cxx/fwd_decl_mapping-i12.h"  // for EnumClass1 (ptr only)
#include "tests/cxx/fwd_decl_mapping-i14.h"  // for EnumClass2
#include "tests/cxx/fwd_decl_mapping-i2.h"  // for OnlyFwdDeclUse (ptr only)
#include "tests/cxx/fwd_decl_mapping-i4.h"  // for FwdDeclAndFullUse
#include "tests/cxx/fwd_decl_mapping-i6.h"  // for FwdDeclInI6 (ptr only), FwdDeclInTwoHeaders1 (ptr only), HasIncludeMapping (ptr only)
#include "tests/cxx/fwd_decl_mapping-i9.h"  // for HasMappingForFullUse (ptr only)

***** IWYU_SUMMARY */
