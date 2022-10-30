//===--- pragma_keep_multi.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// This tests whether or not begin_keep/end_keep works

// Check empty block

// IWYU pragma: begin_keep
// IWYU pragma: end_keep

// Check block with one include

// IWYU pragma: begin_keep
#include "tests/cxx/comment_pragmas-d22.h"
// IWYU pragma: end_keep

// Check block with several includes

// IWYU pragma: begin_keep
#include "tests/cxx/comment_pragmas-d1.h"
#include "tests/cxx/comment_pragmas-d10.h"
#include "tests/cxx/comment_pragmas-d16.h"
#include "tests/cxx/comment_pragmas-d21.h"
// IWYU pragma: end_keep

// Block with one forward decl

// IWYU pragma: begin_keep
class FakeClass;
// IWYU pragma: end_keep

// Block with several forward decls

// IWYU pragma: begin_keep
class FakeClass2;
class FakeClass3;
class FakeClass4;
// IWYU pragma: end_keep

// Block with mix of includes and fwd decls

// IWYU pragma: begin_keep
#include "tests/cxx/comment_pragmas-d17.h"
#include "tests/cxx/comment_pragmas-d13.h"
#include "tests/cxx/comment_pragmas-d14.h"

class FakeClass5;
class FakeClass6;
class FakeClass7;
// IWYU pragma: end_keep

#include "tests/cxx/comment_pragmas-d9.h"

class AnotherFakeClass;

/**** IWYU_SUMMARY 
tests/cxx/pragma_keep_multi.cc should add these lines:

tests/cxx/pragma_keep_multi.cc should remove these lines:
- #include "tests/cxx/comment_pragmas-d9.h"  // lines XX-XX
- class AnotherFakeClass;  // lines XX-XX

The full include-list for tests/cxx/pragma_keep_multi.cc:
#include "tests/cxx/comment_pragmas-d1.h"
#include "tests/cxx/comment_pragmas-d10.h"
#include "tests/cxx/comment_pragmas-d13.h"
#include "tests/cxx/comment_pragmas-d14.h"
#include "tests/cxx/comment_pragmas-d16.h"
#include "tests/cxx/comment_pragmas-d17.h"
#include "tests/cxx/comment_pragmas-d21.h"
#include "tests/cxx/comment_pragmas-d22.h"
class FakeClass2;  // lines XX-XX
class FakeClass3;  // lines XX-XX
class FakeClass4;  // lines XX-XX
class FakeClass5;  // lines XX-XX
class FakeClass6;  // lines XX-XX
class FakeClass7;  // lines XX-XX
class FakeClass;  // lines XX-XX

***** IWYU_SUMMARY */

