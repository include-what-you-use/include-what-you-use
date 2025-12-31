//===--- associated_skipped.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// This is a half-way test. It lays out an include graph that would cause an
// assertion failure in IWYU (issue #738):
//
// * Main file includes the associated header by two different include names
// * The first include is not classified as an associated header
// * The second is, but is skipped due to include guards
//
// This would add the associated header to the include graph, but not to the set
// of headers to be checked for IWYU, which eventually led to the assertion
// failure. This has since been fixed.
//
// Unfortunately this problem only triggers if IWYU is invoked with an absolute
// path, and the current test framework doesn't support that, so this test would
// not trigger the problem before the patch.
//
// And it also triggers another canonicalization problem, where tests/cxx/x.h
// and x.h are not seen as the same header, so the expected results are not
// strictly ideal (both tests/cxx/associated_skipped.h and associated_skipped.h
// are retained).
//
// But it seems valuable to keep this scenario around when making changes to
// this logic, to make sure it doesn't regress even further.

#include "tests/cxx/associated_skipped-d1.h"
#include "tests/cxx/associated_skipped.h"
#include "associated_skipped.h"

int main() {
  // twice is in the associated header, which is already present, so
  // no diagnostic expected.
  int x = twice(4);

  // IWYU: quad(int) is...*associated_skipped-i1.h
  return quad(2);
}

/**** IWYU_SUMMARY

tests/cxx/associated_skipped.cc should add these lines:
#include "tests/cxx/associated_skipped-i1.h"

tests/cxx/associated_skipped.cc should remove these lines:
- #include "tests/cxx/associated_skipped-d1.h"  // lines XX-XX

The full include-list for tests/cxx/associated_skipped.cc:
#include "tests/cxx/associated_skipped.h"
#include "associated_skipped.h"  // for twice
#include "tests/cxx/associated_skipped-i1.h"  // for quad

***** IWYU_SUMMARY */
