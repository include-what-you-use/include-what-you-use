//===--- defn_is_use.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "defn_is_use.h"
#include "defn_is_use-decl.h"
#include "defn_is_use-namespace.h"

void ns1::ns2::Foo() {
}

void SomeFunction() {
}

// IWYU should keep these forward-declarations because they are needed
// to compile the definitions below.
namespace ns1 {
class ClassInNs1;
enum class EnumInNs1;

template <typename>
class Tpl;
// TODO: IWYU should not suggest removing this.
template <typename>
class FriendTpl;
}  // namespace ns1

class ns1::ClassInNs1 {};
enum class ns1::EnumInNs1 {};

// Test that IWYU doesn't crash on a template specialization declaration.
template <>
class ns1::Tpl<int> {};

template <typename>
class TplHavingFriend {
  template <typename>
  friend class ns1::FriendTpl;
};

// IWYU should suggest adding a forward-declaration of ClassInNs2.
// IWYU: ns1::ns2::ClassInNs2 needs a declaration
class ns1::ns2::ClassInNs2 {};

// The associated header already has a fwd-decl. No need in another one here.
class ns1::ns2::Class2InNs2 {};

/**** IWYU_SUMMARY

tests/cxx/defn_is_use.cc should add these lines:
namespace ns1 { namespace ns2 { class ClassInNs2; } }

tests/cxx/defn_is_use.cc should remove these lines:
- namespace ns1 { template <typename> class FriendTpl; }  // lines XX-XX+1

The full include-list for tests/cxx/defn_is_use.cc:
#include "defn_is_use.h"
#include "defn_is_use-decl.h"  // for SomeFunction
#include "defn_is_use-namespace.h"  // for Foo
namespace ns1 { class ClassInNs1; }  // lines XX-XX
namespace ns1 { enum class EnumInNs1; }  // lines XX-XX
namespace ns1 { namespace ns2 { class ClassInNs2; } }
namespace ns1 { template <typename> class Tpl; }  // lines XX-XX+1

***** IWYU_SUMMARY */
