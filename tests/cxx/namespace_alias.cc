//===--- namespace_alias.cc - test input file for IWYU --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Check that using a namespace alias requires an appropriate include

#include "tests/cxx/namespace_alias-d1.h"

// The referenced namespace must be declared. This file doesn't use symbols from
// the namespace, so a header is explicitly suggested for it.
// IWYU: ns3::ns4 is defined in...*namespace_alias-i3.h
namespace ns_alias2 = ns3::ns4;

void Func() {
  // IWYU: ns1::ns2::Function1 is...*namespace_alias-i1.h
  // IWYU: ns_alias1 is defined in...*namespace_alias-i2.h
  ns_alias1::Function1();
}

/**** IWYU_SUMMARY

tests/cxx/namespace_alias.cc should add these lines:
#include "tests/cxx/namespace_alias-i1.h"
#include "tests/cxx/namespace_alias-i2.h"
#include "tests/cxx/namespace_alias-i3.h"

tests/cxx/namespace_alias.cc should remove these lines:
- #include "tests/cxx/namespace_alias-d1.h"  // lines XX-XX

The full include-list for tests/cxx/namespace_alias.cc:
#include "tests/cxx/namespace_alias-i1.h"  // for Function1
#include "tests/cxx/namespace_alias-i2.h"  // for ns_alias1
#include "tests/cxx/namespace_alias-i3.h"  // for ns4

***** IWYU_SUMMARY */
