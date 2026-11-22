//===--- system_namespaces.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that we correctly replace forward declares with #includes for
// items in namespace std.

#include "tests/cxx/system_namespaces-d1.h"
#include "tests/cxx/system_namespaces-d2.h"
#include "tests/cxx/system_namespaces-d3.h"
#include "tests/cxx/system_namespaces-d4.h"

std::StdClass* foo = 0;

__gnu_cxx::SystemClass* bar = 0;

typedef int __system_type;
notsys_ns::TplClass<__system_type>* baz = 0;

// Test that IWYU doesn't crash on remapping __normal_iterator to template
// specialization reference type when handling the outer template instantiation.

template <typename T>
class Tpl {
  T t;
};

Tpl<int> var_for_tpl_int_instantiation;

Tpl<__gnu_cxx::__normal_iterator<char*, Tpl<int>&>> t;

// Test that operators of the provided __normal_iterator are not remapped
// to std::array.
void TestIterator() {
  ArrayDerived ad;
  // IWYU should not suggest <array> here.
  for (int i : ad)
    ;
}

// Besides ArrayDerived which is fully used, only std::StdClass
// should be not replaceable by a forward-declare, because only it is
// exposing a symbol in the std namespace.

/**** IWYU_SUMMARY

tests/cxx/system_namespaces.cc should add these lines:
namespace __gnu_cxx { class SystemClass; }
namespace notsys_ns { template <typename T> class TplClass; }

tests/cxx/system_namespaces.cc should remove these lines:
- #include "tests/cxx/system_namespaces-d2.h"  // lines XX-XX
- #include "tests/cxx/system_namespaces-d3.h"  // lines XX-XX

The full include-list for tests/cxx/system_namespaces.cc:
#include "tests/cxx/system_namespaces-d1.h"  // for StdClass
#include "tests/cxx/system_namespaces-d4.h"  // for ArrayDerived, __normal_iterator (ptr only)
namespace __gnu_cxx { class SystemClass; }
namespace notsys_ns { template <typename T> class TplClass; }

***** IWYU_SUMMARY */
