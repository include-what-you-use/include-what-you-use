//===--- ptr_ref_aliases-d1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/ptr_ref_aliases-i1.h"

struct Indirect;

using NonProvidingPtrAlias = Indirect*;
using NonProvidingRefAlias = Indirect&;

using NonProvidingDoubleRefAlias = NonProvidingRefAlias&;
using NonProvidingDoublePtrAlias = NonProvidingPtrAlias*;
using NonProvidingRefPtrAlias = NonProvidingPtrAlias&;
using NonProvidingDoubleRefPtrAlias = NonProvidingRefPtrAlias&;

/**** IWYU_SUMMARY

tests/cxx/ptr_ref_aliases-d1.h should add these lines:

tests/cxx/ptr_ref_aliases-d1.h should remove these lines:
- #include "tests/cxx/ptr_ref_aliases-i1.h"  // lines XX-XX

The full include-list for tests/cxx/ptr_ref_aliases-d1.h:
struct Indirect;  // lines XX-XX

***** IWYU_SUMMARY */
