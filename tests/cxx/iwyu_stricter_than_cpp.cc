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
#include "tests/cxx/iwyu_stricter_than_cpp-typedefs.h"
#include "tests/cxx/iwyu_stricter_than_cpp-type_alias.h"
#include "tests/cxx/iwyu_stricter_than_cpp-autocast.h"
#include "tests/cxx/iwyu_stricter_than_cpp-fnreturn.h"
// We include this so the second declaration of TwiceDeclaredFunction
// is visible in the translation unit (but not by -d2.h)
#include "tests/cxx/iwyu_stricter_than_cpp-autocast2.h"
#include "tests/cxx/iwyu_stricter_than_cpp-d2.h"
#include "tests/cxx/iwyu_stricter_than_cpp-d3.h"

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

  // Nested name testing
  IndirectStruct3ProvidingTypedef::IndirectClassProvidingTypedef pp;
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4ProvidingTypedef::IndirectClassNonProvidingTypedef pn;
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  IndirectStruct3NonProvidingTypedef::IndirectClassProvidingTypedef np;
  // IWYU: IndirectStruct4 is...*iwyu_stricter_than_cpp-i4.h
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4NonProvidingTypedef::IndirectClassNonProvidingTypedef nn;

  // TODO(csilvers): test template types where we need some (but not
  // all) of the template args as well.
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

  // Nested name testing
  IndirectStruct3ProvidingAl::IndirectClassProvidingAl pp;
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4ProvidingAl::IndirectClassNonProvidingAl pn;
  // IWYU: IndirectStruct3 is...*iwyu_stricter_than_cpp-i3.h
  IndirectStruct3NonProvidingAl::IndirectClassProvidingAl np;
  // IWYU: IndirectStruct4 is...*iwyu_stricter_than_cpp-i4.h
  // IWYU: IndirectClass is...*indirect.h
  IndirectStruct4NonProvidingAl::IndirectClassNonProvidingAl nn;
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
}

/**** IWYU_SUMMARY

tests/cxx/iwyu_stricter_than_cpp.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i4.h"
struct DirectStruct1;
struct DirectStruct2;
struct IndirectStruct1;
struct IndirectStructForwardDeclaredInD1;
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
#include "tests/cxx/iwyu_stricter_than_cpp-fnreturn.h"  // for DoesEverythingRightFn, DoesNotForwardDeclareAndIncludesFn, DoesNotForwardDeclareFn, DoesNotForwardDeclareProperlyFn, IncludesFn, TplDoesEverythingRightAgainFn, TplDoesEverythingRightFn, TplDoesNotForwardDeclareAndIncludesFn, TplDoesNotForwardDeclareFn, TplDoesNotForwardDeclareProperlyFn, TplIncludesFn
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"  // for IndirectStruct2, TplIndirectStruct2
#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"  // for IndirectStruct3
#include "tests/cxx/iwyu_stricter_than_cpp-i4.h"  // for IndirectStruct4
#include "tests/cxx/iwyu_stricter_than_cpp-type_alias.h"  // for DoesEverythingRightAl, DoesNotForwardDeclareAl, DoesNotForwardDeclareAndIncludesAl, DoesNotForwardDeclareProperlyAl, IncludesAl, IncludesElaboratedAl, IndirectStruct3NonProvidingAl, IndirectStruct4NonProvidingAl, TplDoesEverythingRightAgainAl, TplDoesEverythingRightAl, TplDoesNotForwardDeclareAl, TplDoesNotForwardDeclareAndIncludesAl, TplDoesNotForwardDeclareProperlyAl, TplIncludesAl
#include "tests/cxx/iwyu_stricter_than_cpp-typedefs.h"  // for DoesEverythingRight, DoesNotForwardDeclare, DoesNotForwardDeclareAndIncludes, DoesNotForwardDeclareProperly, Includes, IncludesElaborated, IndirectStruct3NonProvidingTypedef, IndirectStruct4NonProvidingTypedef, TplDoesEverythingRight, TplDoesEverythingRightAgain, TplDoesNotForwardDeclare, TplDoesNotForwardDeclareAndIncludes, TplDoesNotForwardDeclareProperly, TplIncludes
struct DirectStruct1;
struct DirectStruct2;
struct IndirectStruct1;
struct IndirectStructForwardDeclaredInD1;
template <typename T> struct TplDirectStruct1;
template <typename T> struct TplDirectStruct2;
template <typename T> struct TplIndirectStruct1;
template <typename T> struct TplIndirectStructForwardDeclaredInD1;

***** IWYU_SUMMARY */
