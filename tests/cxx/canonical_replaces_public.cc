//===--- canonical_replaces_public.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --mapping_file=tests/cxx/canonical_replaces_public.imp

// Tests that every included header should be canonical for at least one
// of the symbols used in a file. Canonical headers are the first mapped header
// and headers include-mapped to it.

#include "tests/cxx/canonical_replaces_public-d1.h"
#include "tests/cxx/canonical_replaces_public-d2.h"
#include "tests/cxx/canonical_replaces_public-d3.h"
#include "tests/cxx/canonical_replaces_public-d4.h"
#include "tests/cxx/canonical_replaces_public-d5.h"
#include "tests/cxx/canonical_replaces_public-d6.h"

// Although '-d1.h' is a public header for Class1, it is not canonical, so it
// should be replaced with '-i2.h'.
// IWYU: Class1 is...*canonical_replaces_public-i2.h
Class1 c1;

// '-d2.h' is considered to be canonical for Class2 because it has a public-to-
// public mapping to '-i3.h'. This models, among other cases, "umbrella headers"
// like boost/variant.hpp.
Class2 c2;

// IWYU should suggest '-i4.h' and '-i5.h' for these and not their common public
// header '-i6.h' because it is canonical for none of them.
// IWYU: Class3 is...*canonical_replaces_public-i4.h
Class3 c3;
// IWYU: Class4 is...*canonical_replaces_public-i5.h
Class4 c4;

// '-d3.h' is considered canonical for Class6 and is public for Class5, so it is
// sufficient.
Class5 c5;
Class6 c6;

// Test minimizing '#include's. '-i10.h' is canonical for Class8 and public for
// Class7, so it should be chosen regardless of Class7 use count.
// IWYU: Class7 is...*canonical_replaces_public-i10.h
Class7 c71;
// IWYU: Class7 is...*canonical_replaces_public-i10.h
Class7 c72;
// IWYU: Class7 is...*canonical_replaces_public-i10.h
Class7 c73;
// IWYU: Class8 is...*canonical_replaces_public-i10.h
Class8 c8;

// Test minimizing '#include's. '-i13.h' is chosen regardless of the count
// of different symbols for which '-i12.h' is canonical.
// IWYU: Class9 is...*canonical_replaces_public-i13.h
Class9 c9;
// IWYU: Class10 is...*canonical_replaces_public-i13.h
Class10 c10;
// IWYU: Class11 is...*canonical_replaces_public-i13.h
Class11 c11;

// IWYU should not suggest removing the directly included canonical header for
// Class13 ('-d4.h') although '-i15.h' provides both Class12 and Class13.
// IWYU: Class12 is...*canonical_replaces_public-i15.h
Class12 c12;
Class13 c13;

// '-d5.h' is considered to be canonical for Class14 fwd-decl uses.
// TODO: IWYU should not complain here.
// IWYU: Class14 needs a declaration
Class14* c14;

// '-i17.h' is canonical (and the only public) for Class15 fwd-decl use and
// public for Class16, but the canonical for Class16 ('-d6.h') is included
// directly, so IWYU should keep it.
// IWYU: Class15 needs a declaration
Class15* c15;
// IWYU: Class16 needs a declaration
Class16* c16;

/**** IWYU_SUMMARY

tests/cxx/canonical_replaces_public.cc should add these lines:
#include "tests/cxx/canonical_replaces_public-i10.h"
#include "tests/cxx/canonical_replaces_public-i13.h"
#include "tests/cxx/canonical_replaces_public-i15.h"
#include "tests/cxx/canonical_replaces_public-i17.h"
#include "tests/cxx/canonical_replaces_public-i2.h"
#include "tests/cxx/canonical_replaces_public-i4.h"
#include "tests/cxx/canonical_replaces_public-i5.h"

tests/cxx/canonical_replaces_public.cc should remove these lines:
- #include "tests/cxx/canonical_replaces_public-d1.h"  // lines XX-XX

The full include-list for tests/cxx/canonical_replaces_public.cc:
#include "tests/cxx/canonical_replaces_public-d2.h"  // for Class2
#include "tests/cxx/canonical_replaces_public-d3.h"  // for Class5, Class6
#include "tests/cxx/canonical_replaces_public-d4.h"  // for Class13
#include "tests/cxx/canonical_replaces_public-d5.h"  // for Class14 (ptr only)
#include "tests/cxx/canonical_replaces_public-d6.h"  // for Class16 (ptr only)
#include "tests/cxx/canonical_replaces_public-i10.h"  // for Class7, Class8
#include "tests/cxx/canonical_replaces_public-i13.h"  // for Class10, Class11, Class9
#include "tests/cxx/canonical_replaces_public-i15.h"  // for Class12
#include "tests/cxx/canonical_replaces_public-i17.h"  // for Class15 (ptr only)
#include "tests/cxx/canonical_replaces_public-i2.h"  // for Class1
#include "tests/cxx/canonical_replaces_public-i4.h"  // for Class3
#include "tests/cxx/canonical_replaces_public-i5.h"  // for Class4

***** IWYU_SUMMARY */
