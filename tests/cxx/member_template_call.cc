//===--- member_template_call.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that a call to a member template with dependent template arguments
// needs the full type of a non-dependent object, before any instantiation.
// Clang keeps such a call as an UnresolvedMemberExpr rather than a MemberExpr.

#include "tests/cxx/member_template_call-d1.h"

template <typename T>
// IWYU: MemberTemplateClass needs a declaration
void ExplicitArgs(const MemberTemplateClass& c) {
  // IWYU: MemberTemplateClass is...*member_template_call-i1.h
  c.Explicit<T>();
}

template <typename T>
// IWYU: MemberTemplateClass needs a declaration
void DeducedArgs(const MemberTemplateClass& c, T t) {
  // IWYU: MemberTemplateClass is...*member_template_call-i1.h
  c.Deduced(t);
}

template <typename T>
// IWYU: MemberTemplateClass needs a declaration
void ViaPointer(const MemberTemplateClass* c) {
  // IWYU: MemberTemplateClass is...*member_template_call-i1.h
  c->Explicit<T>();
}

// A dependent object is the instantiation's responsibility.
template <typename T>
void DependentObject(const T& t) {
  t.template Explicit<int>();
}

// A generic lambda that is never called is still a template inside the
// instantiation of its enclosing function, so the instantiation contains an
// unresolved member call whose object type comes from a template argument.
template <typename C>
void InGenericLambda(C& c) {
  auto l = [&](auto x) { c.template Explicit<decltype(x)>(); };
  (void)l;
}

// IWYU: MemberTemplateClass needs a declaration
void CallInGenericLambda(MemberTemplateClass& c) {
  // IWYU: MemberTemplateClass is...*member_template_call-i1.h
  InGenericLambda(c);
}

/**** IWYU_SUMMARY

tests/cxx/member_template_call.cc should add these lines:
#include "tests/cxx/member_template_call-i1.h"

tests/cxx/member_template_call.cc should remove these lines:
- #include "tests/cxx/member_template_call-d1.h"  // lines XX-XX

The full include-list for tests/cxx/member_template_call.cc:
#include "tests/cxx/member_template_call-i1.h"  // for MemberTemplateClass

***** IWYU_SUMMARY */
