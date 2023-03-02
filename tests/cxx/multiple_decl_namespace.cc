//===--- multiple_decl_namespace.cc - test input file for IWYU ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that we handle multiply declared namespaces correctly.
//
// Where declarations of a namespace occur in multiple headers (either
// because forward declarations were needed or otherwise), IWYU should
// avoid recommending to include a header which is otherwise unused
// when it sees a using directive.
//
// A namespaces' declarations can be scattered around multiple files,
// so there may not be a canonical file to include to satisfy a using
// directive. IWYU should already identify a relevant header to
// include for the namespaced identifier(s) that are being used from
// the namespace.
//
// Where a single file provides all used declarations, IWYU can add
// the namespace to the list of identifiers being provided by the file
// (to be added to the include comment).
//
// Where multiple files provide used declarations, avoid adding the
// namespace to any file.

#include "tests/cxx/multiple_decl_namespace-d2.h"
#include "tests/cxx/multiple_decl_namespace-i1.h"

using namespace test::simple_ns;
// IWYU: test::single_header_ns is...*multiple_decl_namespace-d3.h
using namespace test::single_header_ns;
using namespace test::multiple_header_ns;
// IWYU: test::multiple_header_with_using_ns is...*multiple_decl_namespace-d5.h
using namespace test::multiple_header_with_using_ns;

void uses() {
  // Use an identifier from single_header_ns, declared in d3.h
  // IWYU: single_header_ns::Function3b is...*multiple_decl_namespace-d3.h
  Function3b();

  // Use an identifier whose header will pull in forward declarations
  // in other namespaces. This is declared in d2.h. d2.h includes d1.h
  // that forward declares single_header_ns.
  Function2b();

  // The first declaration of single_header_ns is seen in d1.h, but the
  // declaration that we are using is in d3.h. We should _not_
  // recommend including d1.h for single_header_ns.

  // Use an identifier from multiple_header_ns, declared in d4.h+d6.h.
  // No suggestion for the namespace, rely on symbols used instead.
  // IWYU: multiple_header_ns::Function4b is...*multiple_decl_namespace-d4.h
  Function4b();

  // Use an identifier from multiple_header_with_using_ns, declared in d5.h+d6.h
  // d5.h has a using-directive.
  // IWYU: multiple_header_with_using_ns::Function5b is...*multiple_decl_namespace-d5.h
  Function5b();
}

/**** IWYU_SUMMARY

tests/cxx/multiple_decl_namespace.cc should add these lines:
#include "tests/cxx/multiple_decl_namespace-d3.h"
#include "tests/cxx/multiple_decl_namespace-d4.h"
#include "tests/cxx/multiple_decl_namespace-d5.h"

tests/cxx/multiple_decl_namespace.cc should remove these lines:
- #include "tests/cxx/multiple_decl_namespace-i1.h"  // lines XX-XX

The full include-list for tests/cxx/multiple_decl_namespace.cc:
#include "tests/cxx/multiple_decl_namespace-d2.h"  // for Function2b, simple_ns
#include "tests/cxx/multiple_decl_namespace-d3.h"  // for Function3b, single_header_ns
#include "tests/cxx/multiple_decl_namespace-d4.h"  // for Function4b
#include "tests/cxx/multiple_decl_namespace-d5.h"  // for Function5b, multiple_header_with_using_ns

***** IWYU_SUMMARY */
