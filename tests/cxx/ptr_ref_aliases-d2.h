//===--- ptr_ref_aliases-d2.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/ptr_ref_aliases-i1.h"

// IWYU: Indirect is...*ptr_ref_aliases-i2.h
using ProvidingPtrAlias = Indirect*;
// IWYU: Indirect is...*ptr_ref_aliases-i2.h
using ProvidingRefAlias = Indirect&;

// IWYU: IndirectTpl needs a declaration
// IWYU: Indirect is...*ptr_ref_aliases-i2.h
// IWYU: IndirectTpl is...*ptr_ref_aliases-i2.h
using ProvidingTplPtrAlias = IndirectTpl<Indirect>*;
// IWYU: IndirectTpl needs a declaration
// IWYU: Indirect is...*ptr_ref_aliases-i2.h
// IWYU: IndirectTpl is...*ptr_ref_aliases-i2.h
using ProvidingTplRefAlias = IndirectTpl<Indirect>&;

// IWYU: TplWithProvidedDefArg is...*ptr_ref_aliases-i2.h
using TplWithProvidedDefArgPtrAlias = TplWithProvidedDefArg<>*;
// IWYU: TplWithProvidedDefArg is...*ptr_ref_aliases-i2.h
using TplWithProvidedDefArgRefAlias = TplWithProvidedDefArg<>&;

/**** IWYU_SUMMARY

tests/cxx/ptr_ref_aliases-d2.h should add these lines:
#include "tests/cxx/ptr_ref_aliases-i2.h"

tests/cxx/ptr_ref_aliases-d2.h should remove these lines:
- #include "tests/cxx/ptr_ref_aliases-i1.h"  // lines XX-XX

The full include-list for tests/cxx/ptr_ref_aliases-d2.h:
#include "tests/cxx/ptr_ref_aliases-i2.h"  // for Indirect, IndirectTpl, TplWithProvidedDefArg

***** IWYU_SUMMARY */
