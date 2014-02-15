//===--- include_with_using.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that if we use a symbol from a .h file that the .h file has
// a 'using' declaration for, we don't consider replacing the use with
// a forward-declaration.  On the other hand, if we don't depend on
// the using declaration, replacing is fair game.

// TODO(csilvers): also test UsingDirectiveDecl ('using namespace std').
// TODO(csilvers): also test namespace aliases ('namespace b = std').

#include "tests/cxx/include_with_using-d1.h"
#include "tests/cxx/include_with_using-d2.h"
#include "tests/cxx/include_with_using-d3.h"
#include "tests/cxx/include_with_using-d3b.h"
#include "tests/cxx/include_with_using-d4.h"
#include "tests/cxx/include_with_using-d5.h"
#include "tests/cxx/include_with_using-d5b.h"

PtrInNs* pin = 0;

UsedFromD2* ufd2 = 0;

// Needs the using decl from d3b (but not the type decl from d3!).
PtrInNs3* pin3 = 0;

namespace ns_cc {     // thus, we need the using-decl from d5b.h, not d5.h
namespace internal {
PtrInNs5* pin5 = 0;
}
}

void TestDeclFromD4() {
  // Needs the using decl from d4.
  var_in_d4 = 0;
}

// IWYU: i1::UsingInCc needs a declaration
using i1::UsingInCc;

void TestUsingInCc() {
  // IWYU: UsingInCc is...*include_with_using-i1.h
  UsingInCc uicc;
}


/**** IWYU_SUMMARY

tests/cxx/include_with_using.cc should add these lines:
#include "tests/cxx/include_with_using-i1.h"
class UsedFromD2;
namespace ns3 { class PtrInNs3; }
namespace ns5 { class PtrInNs5; }

tests/cxx/include_with_using.cc should remove these lines:
- #include "tests/cxx/include_with_using-d2.h"  // lines XX-XX
- #include "tests/cxx/include_with_using-d3.h"  // lines XX-XX
- #include "tests/cxx/include_with_using-d5.h"  // lines XX-XX

The full include-list for tests/cxx/include_with_using.cc:
#include "tests/cxx/include_with_using-d1.h"  // for PtrInNs
#include "tests/cxx/include_with_using-d3b.h"  // for PtrInNs3
#include "tests/cxx/include_with_using-d4.h"  // for var_in_d4
#include "tests/cxx/include_with_using-d5b.h"  // for PtrInNs5
#include "tests/cxx/include_with_using-i1.h"  // for UsingInCc
class UsedFromD2;
namespace ns3 { class PtrInNs3; }
namespace ns5 { class PtrInNs5; }

***** IWYU_SUMMARY */
