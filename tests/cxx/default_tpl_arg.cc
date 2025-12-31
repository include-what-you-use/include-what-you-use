//===--- default_tpl_arg.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also="tests/cxx/*-d2.h"

// Tests handling default template arguments. In particular, that IWYU
// doesn't crash when they refer to uninstantiated template specializations.

#include "tests/cxx/default_tpl_arg-d1.h"
#include "tests/cxx/default_tpl_arg-d2.h"
#include "tests/cxx/direct.h"

// IWYU: UninstantiatedTpl needs a declaration
// IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
template <typename = UninstantiatedTpl<int>>
struct Tpl {};

template <typename T>
struct Outer1 {
  // IWYU: UninstantiatedTpl needs a declaration
  // IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
  template <typename = UninstantiatedTpl<T>>
  struct Inner {};
};

Outer1<int> o1;

template <typename T1, typename T2>
struct Outer2 {
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  template <typename = IndirectTemplate<T1>>
  struct Inner {};
};

// Test that IWYU should not suggest to provide default template argument
// of an internal template on instantiation side.
// IWYU: IndirectTemplate needs a declaration
Outer2<int, IndirectTemplate<int>> o2;

template <typename = int, typename>
// IWYU: ClassTpl is...*default_tpl_arg-i1.h
class ClassTpl;

// Test that IWYU doesn't remap the required declaration to the definition and
// hence reports it here.
template <typename = int, typename>
// IWYU: ClassTplWithDefinition is...*default_tpl_arg-i1.h
class ClassTplWithDefinition {};

template <typename = int, typename>
// IWYU: VarTpl is...*default_tpl_arg-i1.h
extern int VarTpl;

// No need to report anything here.
template <typename, typename>
using AliasTpl1 = int;
template <int, int>
using AliasTpl2 = int;
template <template <int> typename, template <int> typename>
using AliasTpl3 = int;

template <typename = int, typename>
// IWYU: AliasTpl1 is...*default_tpl_arg-i1.h
using AliasTpl1 = int;

template <typename = int, typename>
void FnTpl();

template <int = 0, int>
// IWYU: AliasTpl2 is...*default_tpl_arg-i1.h
using AliasTpl2 = int;

// IWYU: SomeTpl is...*default_tpl_arg-i1.h
template <template <int> typename = SomeTpl, template <int> typename>
// IWYU: AliasTpl3 is...*default_tpl_arg-i1.h
using AliasTpl3 = int;

template <typename = int, int>
// IWYU: AliasTpl4 is...*default_tpl_arg-i1.h
using AliasTpl4 = int;

// The needed redeclaration is in a directly included file.
template <int = 0, int, int>
using AliasTpl5 = int;

// This redeclaration doesn't require any previous declaration to be present.
template <int, int = 0>
using AliasTpl6 = int;

// The required redeclaration is the previous one.
template <int = 0, int>
using AliasTpl6 = int;

// Just to avoid suggestion to remove unused forward-declaration.
ClassTpl<>* class_tpl_ptr;

// TODO: no need to report in these two cases because the redeclaration that is
// already present in this file should "pull" the needed one.
// IWYU: ClassTpl is...*default_tpl_arg-i1.h
ClassTpl<int>* sufficient_redecl_present_in_this_file1;
// IWYU: ClassTpl is...*default_tpl_arg-i1.h
ClassTpl<int, int>* sufficient_redecl_present_in_this_file2;

// IWYU: SpecializedClassTpl is...*default_tpl_arg-i1.h
SpecializedClassTpl<int>* pscti;

template <>
// IWYU: SpecializedClassTpl is...*default_tpl_arg-i1.h
class SpecializedClassTpl<int> {};

// IWYU: ClassTplNoDefinition is...*default_tpl_arg-i1.h
ClassTplNoDefinition<>* p_class_tpl_no_def;

// IWYU: ClassTpl2 is...*default_tpl_arg-i1.h
ClassTpl2<>* use_i1_class_tpl_decl;
// IWYU: ClassTpl2 is...*default_tpl_arg-i2.h
ClassTpl2<int>* use_i2_class_tpl_decl;
// TODO: a forward-declaration is allowed for this case.
// IWYU: ClassTpl2 is...*default_tpl_arg-i2.h
ClassTpl2<int, int>* all_class_tpl_args_specified_explicitly;

// IWYU: ClassTplWithDefinition2 is...*default_tpl_arg-i1.h
ClassTplWithDefinition2<>* fwd_decl_use_cls_tpl_with_def1;
// IWYU: ClassTplWithDefinition2 needs a declaration
ClassTplWithDefinition2<int>* fwd_decl_use_cls_tpl_with_def2;
// "*-i1.h" for the default argument, "*-i2.h" for the definition.
// IWYU: ClassTplWithDefinition2 is...*default_tpl_arg-i1.h
// IWYU: ClassTplWithDefinition2 is...*default_tpl_arg-i2.h
ClassTplWithDefinition2<> full_use_cls_tpl_with_def1;
// IWYU: ClassTplWithDefinition2 is...*default_tpl_arg-i2.h
ClassTplWithDefinition2<int> full_use_cls_tpl_with_def2;

// Test that IWYU doesn't suggest removing these as unused fwd-declarations.
template <typename = int>
class ClassTpl3;
template <typename = int>
class ClassTplWithDefinition3;

ClassTpl3<>* class_tpl_3_fwd_decl_use;
// IWYU: ClassTplWithDefinition3 is...*default_tpl_arg-i2.h
ClassTplWithDefinition3<> cls_tpl_with_def_3_full_use;

// IWYU: ClassTpl4 needs a declaration
ClassTpl4<int>* cls_tpl_4_fwd_decl_use;
// Test correct form of the forward-declaration in the IWYU summary below.
// IWYU: ClassTplWithDefinition4 needs a declaration
ClassTplWithDefinition4<int>* cls_tpl_with_def_4_fwd_decl_use;
// IWYU: ClassTplWithDefinition5 needs a declaration
ClassTplWithDefinition5<int>* cls_tpl_with_def_5_fwd_decl_use;

template <typename>
class ClassTplWithDefinition6;
ClassTplWithDefinition6<int>* cls_tpl_with_def_6_fwd_decl_use;

// IWYU: ClassTplWithDefinition7 is...*default_tpl_arg-i2.h
ClassTplWithDefinition7<> cls_tpl_with_def_7_full_use1;
// IWYU: ClassTplWithDefinition7 is...*default_tpl_arg-i2.h
ClassTplWithDefinition7<>* cls_tpl_with_def_7_fwd_decl_use1;
// IWYU: ClassTplWithDefinition7 is...*default_tpl_arg-i2.h
ClassTplWithDefinition7<char> cls_tpl_with_def_7_full_use2;
// IWYU: ClassTplWithDefinition7 needs a declaration
ClassTplWithDefinition7<char>* cls_tpl_with_def_7_fwd_decl_use2;

// IWYU: AliasTpl7 is...*default_tpl_arg-i1.h
AliasTpl7<> i1;
// IWYU: AliasTpl7 is...*default_tpl_arg-i2.h
AliasTpl7<int> i2;
// IWYU: AliasTpl7 is...*default_tpl_arg-i2.h
AliasTpl7<int, int> i3;
// IWYU: AliasTpl7 is...*default_tpl_arg-i1.h
AliasTpl7<>* pi1;
// IWYU: AliasTpl7 is...*default_tpl_arg-i2.h
AliasTpl7<int>* pi2;
// IWYU: AliasTpl7 is...*default_tpl_arg-i2.h
AliasTpl7<int, int>* pi3;

AliasTpl1<> i4;
// TODO: no need to report here because the required declaration should be
// already "pulled" by the one that specifies the second default argument and
// is present in this file above.
// IWYU: AliasTpl1 is...*default_tpl_arg-i1.h
AliasTpl1<int> i5;
AliasTpl1<int, int> i6;

void Fn() {
  // IWYU: FnWithNonProvidedDefaultTplArg() is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)&FnWithNonProvidedDefaultTplArg<>;
  // IWYU: FnWithNonProvidedDefaultTplArg() is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  FnWithNonProvidedDefaultTplArg();

  (void)&FnWithProvidedDefaultTplArg<>;
  FnWithProvidedDefaultTplArg();

  // IWYU: IndirectClass is...*indirect.h
  using ProvidingAlias = IndirectClass;
  ProvidingAlias* p = 0;
  // IWYU: NonProvidingAlias is...*default_tpl_arg-i1.h
  NonProvidingAlias* n = 0;

  // IWYU: FnWithNonProvidedDefaultTplArgAndDefaultCallArg(:0 *) is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  FnWithNonProvidedDefaultTplArgAndDefaultCallArg();
  FnWithProvidedDefaultTplArgAndDefaultCallArg1();

  // IWYU: FnWithNonProvidedDefaultTplArgAndDefaultCallArg(:0 *) is...*default_tpl_arg-i1.h
  FnWithNonProvidedDefaultTplArgAndDefaultCallArg(p);
  FnWithProvidedDefaultTplArgAndDefaultCallArg1(p);

  // IWYU: FnWithNonProvidedDefaultTplArgAndDefaultCallArg(:0 *) is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  FnWithNonProvidedDefaultTplArgAndDefaultCallArg(n);
  // IWYU: IndirectClass is...*indirect.h
  FnWithProvidedDefaultTplArgAndDefaultCallArg1(n);

  // Some additional variations of default arguments. Should not report
  // IndirectClass because it is provided by the templates.
  FnWithProvidedDefaultTplArgAndDefaultCallArg2();
  FnWithProvidedDefaultTplArgAndDefaultCallArg3();

  auto& class_tpl_2_ref = GetClassTpl2Ref();
  // No need to report anything here.
  [&class_tpl_2_ref] {}();
}

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg.cc should add these lines:
#include "tests/cxx/default_tpl_arg-i1.h"
#include "tests/cxx/default_tpl_arg-i2.h"
#include "tests/cxx/indirect.h"
template <typename> class ClassTpl4;
template <typename> class ClassTplWithDefinition4;
template <typename> class ClassTplWithDefinition5;

tests/cxx/default_tpl_arg.cc should remove these lines:
- #include "tests/cxx/default_tpl_arg-d1.h"  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg.cc:
#include "tests/cxx/default_tpl_arg-d2.h"  // for AliasTpl5, FnWithProvidedDefaultTplArg, FnWithProvidedDefaultTplArgAndDefaultCallArg1, FnWithProvidedDefaultTplArgAndDefaultCallArg2, FnWithProvidedDefaultTplArgAndDefaultCallArg3, GetClassTpl2Ref
#include "tests/cxx/default_tpl_arg-i1.h"  // for AliasTpl1, AliasTpl2, AliasTpl3, AliasTpl4, AliasTpl7, ClassTpl, ClassTpl2, ClassTplNoDefinition, ClassTplWithDefinition, ClassTplWithDefinition2, FnWithNonProvidedDefaultTplArg, FnWithNonProvidedDefaultTplArgAndDefaultCallArg, NonProvidingAlias, SomeTpl, SpecializedClassTpl, UninstantiatedTpl, VarTpl
#include "tests/cxx/default_tpl_arg-i2.h"  // for AliasTpl7, ClassTpl2, ClassTplWithDefinition2, ClassTplWithDefinition3, ClassTplWithDefinition7
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
template <typename = int, typename> class ClassTpl;  // lines XX-XX+2
template <typename = int> class ClassTpl3;  // lines XX-XX+1
template <typename = int> class ClassTplWithDefinition3;  // lines XX-XX+1
template <typename> class ClassTpl4;
template <typename> class ClassTplWithDefinition4;
template <typename> class ClassTplWithDefinition5;
template <typename> class ClassTplWithDefinition6;  // lines XX-XX+1

***** IWYU_SUMMARY */
