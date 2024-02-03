//===--- iwyu_stricter_than_cpp.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/*-autocast.h" \
//            -Xiwyu --check_also="tests/cxx/*-fnreturn.h" \
//            -Xiwyu --check_also="tests/cxx/*-typedefs.h" \
//            -Xiwyu --check_also="tests/cxx/*-type_alias.h" \
//            -Xiwyu --check_also="tests/cxx/*-def_tpl_arg.h" \
//            -Xiwyu --check_also="tests/cxx/*-d2.h" \
//            -I .

// There are a few scenarios where iwyu requires a full type but c++
// doesn't.
//
// One is in a typedef: if you write 'typedef Foo MyTypedef', iwyu
// says that you are responsible for #including "foo.h", but the
// language allows a forward-declare.
//
// Another is for 'autocast': if your function has a parameter with a
// conversion (one-arg, not-explicit) constructor, iwyu require the
// function-author to provide the full type of that parameter, but the
// language doesn't.  (It's ok with all callers providing the full
// type instead.)
//
// In each case, we can disable iwyu's rule, and force it to fall back
// on the c++ requirement (forward-declare ok), by writing the code in
// the following way:
// (1) forward-declare the relevant type
// (2) do not directly #include the definition of the relevant type.
//
// This test tests that the iwyu requirement is correctly suppressed
// when these two conditions are met, and not otherwise.

#include "tests/cxx/direct.h"
#include "tests/cxx/iwyu_stricter_than_cpp-def_tpl_arg.h"
#include "tests/cxx/iwyu_stricter_than_cpp-typedefs.h"
#include "tests/cxx/iwyu_stricter_than_cpp-type_alias.h"
#include "tests/cxx/iwyu_stricter_than_cpp-autocast.h"
#include "tests/cxx/iwyu_stricter_than_cpp-fnreturn.h"
// We include this so the second declaration of TwiceDeclaredFunction
// is visible in the translation unit (but not by -d2.h)
#include "tests/cxx/iwyu_stricter_than_cpp-autocast2.h"
#include "tests/cxx/iwyu_stricter_than_cpp-d2.h"
#include "tests/cxx/iwyu_stricter_than_cpp-d3.h"
#include "tests/cxx/iwyu_stricter_than_cpp-d5.h"

template <typename T>
void UsingFn() {
  T t;
}

template <typename T>
void NonUsingFn() {
  T* t = nullptr;
}

template <typename T>
struct UsingType {
  T t;
};

template <typename T>
struct NonUsingType {
  T* t = nullptr;
};

template <typename T1, typename T2>
struct WithAlias {
  // IWYU: TplIndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  using Type = TplIndirectStruct2<T1>;
};

typedef DoesEverythingRight DoubleTypedef;

// If the typedef in -typedefs.h requires the full type, then users of
// that typedef (here) do not.  Otherwise, they do.
void TestTypedefs() {
  DoesNotForwardDeclare dnfd(1);
  DoesNotForwardDeclareProperly dnfdp(2);
  Includes i(3);
  IncludesElaborated ie(3);
  DoesNotForwardDeclareAndIncludes dnfdai(4);
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  DoesEverythingRight dor(5);
  // Because DoubleTypedef resolves to DoesEverythingRight, we need the
  // same things DoesEverythingRight does.
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  DoubleTypedef dt(6);

  // ...and with templates.
  TplDoesNotForwardDeclare tdnfd(7);
  TplDoesNotForwardDeclareProperly tdnfdp(8);
  TplIncludes ti(9);
  TplDoesNotForwardDeclareAndIncludes tdnfdai(10);
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplDoesEverythingRight tdor(11);
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplDoesEverythingRightAgain tdora(12);

  // But if we're in a forward-declare context, we don't require the
  // underlying type!
  DoesEverythingRight* dor_ptr = 0;
  TplDoesEverythingRightAgain* tdora_ptr = 0;
  // ...at least until we dereference the pointer
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)dor_ptr->a;

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  TplOnlyArgumentTypeProvided toatp;
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplAllForwardDeclared tafd;
  TplAllNeededTypesProvided tantp;
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplOnlyTemplateProvided totp;

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  UsingFn<TplOnlyArgumentTypeProvided>();
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingFn<TplAllForwardDeclared>();
  UsingFn<TplAllNeededTypesProvided>();
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingFn<TplOnlyTemplateProvided>();

  NonUsingFn<TplAllForwardDeclared>();

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  UsingType<TplOnlyArgumentTypeProvided> only_argument_provided;
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingType<TplAllForwardDeclared> all_fwd_declared;
  UsingType<TplAllNeededTypesProvided> all_provided;
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingType<TplOnlyTemplateProvided> only_template_provided;

  NonUsingType<TplAllForwardDeclared> no_type_needed;

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  (void)sizeof(only_argument_provided);
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)sizeof(all_fwd_declared);
  (void)sizeof(all_provided);
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)sizeof(only_template_provided);

  // Nested name testing
  IndirectStruct3ProvidingTypedef::IndirectClassProvidingTypedef pp;
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4ProvidingTypedef::IndirectClassNonProvidingTypedef pn;
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  IndirectStruct3NonProvidingTypedef::IndirectClassProvidingTypedef np;
  // IWYU: IndirectStruct4 is...*iwyu_stricter_than_cpp-i4.h
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4NonProvidingTypedef::IndirectClassNonProvidingTypedef nn;
}

using DoubleTypedefAl = DoesEverythingRightAl;

void TestTypeAliases() {
  DoesNotForwardDeclareAl dnfd(1);
  DoesNotForwardDeclareProperlyAl dnfdp(2);
  IncludesAl i(3);
  IncludesElaboratedAl ie(3);
  DoesNotForwardDeclareAndIncludesAl dnfdai(4);
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  DoesEverythingRightAl dor(5);
  // Because DoubleTypedefAl resolves to DoesEverythingRightAl, we need the
  // same things DoesEverythingRightAl does.
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  DoubleTypedefAl dt(6);

  // ...and with templates.
  TplDoesNotForwardDeclareAl tdnfd(7);
  TplDoesNotForwardDeclareProperlyAl tdnfdp(8);
  TplIncludesAl ti(9);
  TplDoesNotForwardDeclareAndIncludesAl tdnfdai(10);
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplDoesEverythingRightAl tdor(11);
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplDoesEverythingRightAgainAl tdora(12);

  // But if we're in a forward-declare context, we don't require the
  // underlying type!
  DoesEverythingRightAl* dor_ptr = 0;
  TplDoesEverythingRightAgainAl* tdora_ptr = 0;
  // ...at least until we dereference the pointer
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)dor_ptr->a;

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  TplOnlyArgumentTypeProvidedAl toatp;
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplAllForwardDeclaredAl tafd;
  TplAllNeededTypesProvidedAl tantp;
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplOnlyTemplateProvidedAl totp;

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  UsingFn<TplOnlyArgumentTypeProvidedAl>();
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingFn<TplAllForwardDeclaredAl>();
  UsingFn<TplAllNeededTypesProvidedAl>();
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingFn<TplOnlyTemplateProvidedAl>();

  NonUsingFn<TplAllForwardDeclaredAl>();

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  UsingType<TplOnlyArgumentTypeProvidedAl> only_argument_provided;
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingType<TplAllForwardDeclaredAl> all_fwd_declared;
  UsingType<TplAllNeededTypesProvidedAl> all_provided;
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  UsingType<TplOnlyTemplateProvidedAl> only_template_provided;

  NonUsingType<TplAllForwardDeclaredAl> no_type_needed;

  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  (void)sizeof(only_argument_provided);
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)sizeof(all_fwd_declared);
  (void)sizeof(all_provided);
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)sizeof(only_template_provided);

  // Nested name testing
  IndirectStruct3ProvidingAl::IndirectClassProvidingAl pp;
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4ProvidingAl::IndirectClassNonProvidingAl pn;
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  IndirectStruct3NonProvidingAl::IndirectClassProvidingAl np;
  // IWYU: IndirectStruct4 is...*iwyu_stricter_than_cpp-i4.h
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4NonProvidingAl::IndirectClassNonProvidingAl nn;

  // Test that IWYU should not suggest to provide underlying type of template
  // internal type alias on instantiation side.
  // IWYU: TplIndirectStruct2 needs a declaration
  WithAlias<int, TplIndirectStruct2<int>> wa;
}

void TestAliasTemplates() {
  DoesNotForwardDeclareAlTpl<int> dnfd(1);
  IncludesAlTpl<int> i(2);
  DoesNotForwardDeclareAndIncludesAlTpl<int> dnfdai(3);
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  DoesEverythingRightAlTpl<int> dor(4);
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  (void)&DoesEverythingRightAlTpl<int>::a;

  // IWYU: IndirectStruct3 needs a declaration
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  TemplateProvidedArgumentUsed<IndirectStruct3> tpau;
  // IWYU: IndirectStruct3 needs a declaration
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  TemplateNotProvidedArgumentUsed<IndirectStruct3> tnpau;
  // IWYU: IndirectStruct3 needs a declaration
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TemplateProvidedArgumentNotUsed<IndirectStruct3> tpanu;
  // IWYU: IndirectStruct3 needs a declaration
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  TemplateNotProvidedArgumentNotUsed<IndirectStruct3> tnpanu;

  TemplateProvidedArgumentUsed<DoesNotForwardDeclareAndIncludes>
      argument_provided;
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TemplateProvidedArgumentUsed<DoesEverythingRight> argument_not_provided;
}

void TestAutocast() {
  // We need full type of IndirectStruct2 because the declarer of the following
  // functions didn't.
  FnValues(1, 2, 3, 4,
           // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
           5);
  FnRefs(1, 2, 3, 4,
         // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
         5);
  HeaderDefinedFnRefs(1, 2, 3, 4,
                      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
                      5);

  // We need full type of TplIndirectStruct2 because the declarer
  // of the following functions didn't.
  TplFnValues(6, 7, 8, 9,
              // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
              10);
  TplFnRefs(6, 7, 8, 9,
            // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
            10);
  HeaderDefinedTplFnRefs(
      6, 7, 8, 9,
      // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      10);

  TplFnValues(
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      /*only_argument_type_provided=*/1,
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      /*all_forward_declared=*/2,
      /*all_needed_types_provided=*/3,
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      /*only_template_provided=*/4);

  TplFnRefs(
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      /*only_argument_type_provided=*/1,
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      /*all_forward_declared=*/2,
      /*all_needed_types_provided=*/3,
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      /*only_template_provided=*/4);

  HeaderDefinedTplFnRefs(
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      /*only_argument_type_provided=*/1,
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      /*all_forward_declared=*/2,
      /*all_needed_types_provided=*/3,
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      /*only_template_provided=*/4);
}

template <typename Ret, Ret Fn()>
void Call() {
  Fn();
}

void TestFunctionReturn() {
  // In each of these cases, we bind the return value to a reference,
  // so we only need a forward-declare unless the function-author has
  // not taken responsibility for the return type.

  // IWYU: IndirectStruct1 needs a declaration
  const IndirectStruct1& is1 = DoesNotForwardDeclareFn();

  // IWYU: IndirectStructForwardDeclaredInD1 needs a declaration
  const IndirectStructForwardDeclaredInD1& isfdid1 =
      DoesNotForwardDeclareProperlyFn();

  // IWYU: DirectStruct1 needs a declaration
  const DirectStruct1& ds1 = IncludesFn();

  // IWYU: DirectStruct2 needs a declaration
  const DirectStruct2& ds2 = DoesNotForwardDeclareAndIncludesFn();

  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  const IndirectStruct2& is2 = DoesEverythingRightFn();

  // -- And with templates.

  // IWYU: TplIndirectStruct1 needs a declaration
  const TplIndirectStruct1<int>& tis1 = TplDoesNotForwardDeclareFn();

  // IWYU: TplIndirectStructForwardDeclaredInD1 needs a declaration
  const TplIndirectStructForwardDeclaredInD1<int>& tisfdid1 =
      TplDoesNotForwardDeclareProperlyFn();

  // IWYU: TplDirectStruct1 needs a declaration
  const TplDirectStruct1<int>& tds1 = TplIncludesFn();

  // IWYU: TplDirectStruct2 needs a declaration
  const TplDirectStruct2<int>& tds2 = TplDoesNotForwardDeclareAndIncludesFn();

  // IWYU: TplIndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  const TplIndirectStruct2<int>& tis2 = TplDoesEverythingRightFn();

  // IWYU: TplIndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  const TplIndirectStruct2<float>& tis2b = TplDoesEverythingRightAgainFn();

  // -- With user-defined types as template parameters.

  // IWYU: IndirectStruct1 needs a declaration
  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct3 needs a declaration
  const TplIndirectStruct3<IndirectStruct1, IndirectStruct2>& tis3a =
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      TplOnlyArgumentTypeProvidedFn();

  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct3 needs a declaration
  const TplIndirectStruct3<IndirectStruct2, IndirectStruct2>& tis3b =
      // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      TplAllForwardDeclaredFn();

  // IWYU: IndirectStruct1 needs a declaration
  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplDirectStruct7 needs a declaration
  const TplDirectStruct7<IndirectStruct1, IndirectStruct2>& tds3a =
      TplAllNeededTypesProvidedFn();

  // IWYU: TplDirectStruct7 needs a declaration
  // IWYU: IndirectStruct2 needs a declaration
  const TplDirectStruct7<IndirectStruct2, IndirectStruct2>& tds3b =
      // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
      TplOnlyTemplateProvidedFn();

  // -- Calls from a template.

  // IWYU: IndirectStruct1 needs a declaration
  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct3 needs a declaration
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  Call<TplIndirectStruct3<IndirectStruct1, IndirectStruct2>,
       TplOnlyArgumentTypeProvidedFn>();
  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplIndirectStruct3 needs a declaration
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  Call<TplIndirectStruct3<IndirectStruct2, IndirectStruct2>,
       TplAllForwardDeclaredFn>();
  // IWYU: IndirectStruct1 needs a declaration
  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplDirectStruct7 needs a declaration
  Call<TplDirectStruct7<IndirectStruct1, IndirectStruct2>,
       TplAllNeededTypesProvidedFn>();
  // IWYU: IndirectStruct2 needs a declaration
  // IWYU: TplDirectStruct7 needs a declaration
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  Call<TplDirectStruct7<IndirectStruct2, IndirectStruct2>,
       TplOnlyTemplateProvidedFn>();

  // -- Call from a template with "providing" alias as a template argument.

  // IWYU: TplIndirectStruct3 needs a declaration
  // IWYU: TplIndirectStruct3 is...*iwyu_stricter_than_cpp-i5.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  using Alias = TplIndirectStruct3<IndirectStruct2, IndirectStruct2>;

  Call<Alias, TplAllForwardDeclaredFn>();

  // IWYU: IndirectClass is...*indirect.h
  RetNonProvidingTypedef();
}

void TestDefaultTplArgs() {
  // There is currently some difference between default type template arguments
  // and the other cases: on the user's side, IWYU requires complete type info
  // when the template defining file doesn't provide the corresponding header
  // (for user-defined templates, it usually means "doesn't include directly")
  // regardless of fwd-decl presence or absence. Hence, 'IndirectStruct3' (which
  // is neither directly included nor fwd-declared in the template defn header)
  // is required both here and at the template definition side.
  // 'IndirectStruct2' is required here because it is fwd-declared and not
  // included directly in the template defining header.
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplWithDefaultArgs<> t;
}

/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i4.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i5.h"
struct DirectStruct1;
struct DirectStruct2;
struct IndirectStruct1;
struct IndirectStructForwardDeclaredInD1;
template <typename T1, typename T2> struct TplDirectStruct7;
template <typename T> struct TplDirectStruct1;
template <typename T> struct TplDirectStruct2;
template <typename T> struct TplIndirectStruct1;
template <typename T> struct TplIndirectStructForwardDeclaredInD1;

tests/cxx/iwyu_stricter_than_cpp.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/iwyu_stricter_than_cpp-autocast2.h"  // lines XX-XX
- #include "tests/cxx/iwyu_stricter_than_cpp-d2.h"  // lines XX-XX

The full include-list for tests/cxx/iwyu_stricter_than_cpp.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/iwyu_stricter_than_cpp-autocast.h"  // for FnRefs, FnValues, HeaderDefinedFnRefs, HeaderDefinedTplFnRefs, TplFnRefs, TplFnValues
#include "tests/cxx/iwyu_stricter_than_cpp-d3.h"  // for IndirectStruct3ProvidingAl, IndirectStruct3ProvidingTypedef, IndirectStruct4ProvidingAl, IndirectStruct4ProvidingTypedef
#include "tests/cxx/iwyu_stricter_than_cpp-d5.h"  // for RetNonProvidingTypedef
#include "tests/cxx/iwyu_stricter_than_cpp-def_tpl_arg.h"  // for TplWithDefaultArgs
#include "tests/cxx/iwyu_stricter_than_cpp-fnreturn.h"  // for DoesEverythingRightFn, DoesNotForwardDeclareAndIncludesFn, DoesNotForwardDeclareFn, DoesNotForwardDeclareProperlyFn, IncludesFn, TplAllForwardDeclaredFn, TplAllNeededTypesProvidedFn, TplDoesEverythingRightAgainFn, TplDoesEverythingRightFn, TplDoesNotForwardDeclareAndIncludesFn, TplDoesNotForwardDeclareFn, TplDoesNotForwardDeclareProperlyFn, TplIncludesFn, TplOnlyArgumentTypeProvidedFn, TplOnlyTemplateProvidedFn
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"  // for IndirectStruct2, TplIndirectStruct2
#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"  // for IndirectStruct3
#include "tests/cxx/iwyu_stricter_than_cpp-i4.h"  // for IndirectStruct4
#include "tests/cxx/iwyu_stricter_than_cpp-i5.h"  // for TplIndirectStruct3
#include "tests/cxx/iwyu_stricter_than_cpp-type_alias.h"  // for DoesEverythingRightAl, DoesEverythingRightAlTpl, DoesNotForwardDeclareAl, DoesNotForwardDeclareAlTpl, DoesNotForwardDeclareAndIncludesAl, DoesNotForwardDeclareAndIncludesAlTpl, DoesNotForwardDeclareProperlyAl, IncludesAl, IncludesAlTpl, IncludesElaboratedAl, IndirectStruct3NonProvidingAl, IndirectStruct4NonProvidingAl, TemplateNotProvidedArgumentNotUsed, TemplateNotProvidedArgumentUsed, TemplateProvidedArgumentNotUsed, TemplateProvidedArgumentUsed, TplAllForwardDeclaredAl, TplAllNeededTypesProvidedAl, TplDoesEverythingRightAgainAl, TplDoesEverythingRightAl, TplDoesNotForwardDeclareAl, TplDoesNotForwardDeclareAndIncludesAl, TplDoesNotForwardDeclareProperlyAl, TplIncludesAl, TplOnlyArgumentTypeProvidedAl, TplOnlyTemplateProvidedAl
#include "tests/cxx/iwyu_stricter_than_cpp-typedefs.h"  // for DoesEverythingRight, DoesNotForwardDeclare, DoesNotForwardDeclareAndIncludes, DoesNotForwardDeclareProperly, Includes, IncludesElaborated, IndirectStruct3NonProvidingTypedef, IndirectStruct4NonProvidingTypedef, TplAllForwardDeclared, TplAllNeededTypesProvided, TplDoesEverythingRight, TplDoesEverythingRightAgain, TplDoesNotForwardDeclare, TplDoesNotForwardDeclareAndIncludes, TplDoesNotForwardDeclareProperly, TplIncludes, TplOnlyArgumentTypeProvided, TplOnlyTemplateProvided
struct DirectStruct1;
struct DirectStruct2;
struct IndirectStruct1;
struct IndirectStructForwardDeclaredInD1;
template <typename T1, typename T2> struct TplDirectStruct7;
template <typename T> struct TplDirectStruct1;
template <typename T> struct TplDirectStruct2;
template <typename T> struct TplIndirectStruct1;
template <typename T> struct TplIndirectStructForwardDeclaredInD1;

***** IWYU_SUMMARY */
