//===--- system_namespaces.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we correctly replace forward declares with #includes for
// items in a system namespace: std or __gnu_cxx or the like.

#include "tests/cxx/system_namespaces-d1.h"
#include "tests/cxx/system_namespaces-d2.h"
#include "tests/cxx/system_namespaces-d3.h"

std::StdClass* foo = 0;

__gnu_cxx::SystemClass* bar = 0;

typedef int __system_type;
notsys_ns::TplClass<__system_type>* baz = 0;


// The summary is where all the action is on this test.  Only TplClass
// should be replaceable by a forward-declare, because only it is
// exposing a symbol in a non-system namespace.

/**** IWYU_SUMMARY

tests/cxx/system_namespaces.cc should add these lines:
namespace notsys_ns { template <typename T> class TplClass; }

tests/cxx/system_namespaces.cc should remove these lines:
- #include "tests/cxx/system_namespaces-d3.h"  // lines XX-XX

The full include-list for tests/cxx/system_namespaces.cc:
#include "tests/cxx/system_namespaces-d1.h"  // for StdClass
#include "tests/cxx/system_namespaces-d2.h"  // for SystemClass
namespace notsys_ns { template <typename T> class TplClass; }

***** IWYU_SUMMARY */
