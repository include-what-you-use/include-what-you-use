//===--- re_fwd_decl.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// If we use a class in a way that only requires a forward-declaration,
// and the class is defined in some file we directly #include, we don't
// need the forward-decl here.  But if it's only *declared* in a file
// we directly #include, we *do* need the forward-decl.

#include "tests/re_fwd_decl.h"
#include "tests/re_fwd_decl-d1.h"

Direct* d = 0;
// IWYU: Indirect needs a declaration
Indirect* i = 0;
// This is not an iwyu violation because it's declared in the .h file
DeclaredInH* dih = 0;

// This forces us to #include re_fwd_decl-d1.h.  Without this, iwyu would
// just suggest replacing the #include with 'class Direct'.
FullUse fu;

/**** IWYU_SUMMARY

tests/re_fwd_decl.cc should add these lines:
class Indirect;

tests/re_fwd_decl.cc should remove these lines:

The full include-list for tests/re_fwd_decl.cc:
#include "tests/re_fwd_decl.h"
#include "tests/re_fwd_decl-d1.h"  // for Direct (ptr only), FullUse
class Indirect;

***** IWYU_SUMMARY */
