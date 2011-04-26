//===--- iwyu_stricter_than_cpp.cc - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

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

#include "tests/iwyu_stricter_than_cpp-typedefs.h"
#include "tests/iwyu_stricter_than_cpp-autocast.h"
#include "tests/iwyu_stricter_than_cpp-fnreturn.h"
// We include this so the second declaration of TwiceDeclaredFunction
// is visible in the translation unit (but not by -d2.h)
#include "tests/iwyu_stricter_than_cpp-autocast2.h"
#include "tests/iwyu_stricter_than_cpp-d2.h"

typedef DoesEverythingRight DoubleTypedef;

// If the typedef in -typedefs.h requires the full type, then users of
// that typedef (here) do not.  Otherwise, they do.
void TestTypedefs() {
  DoesNotForwardDeclare dnfd(1);
  DoesNotForwardDeclareProperly dnfdp(2);
  Includes i(3);
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

  // TODO(csilvers): test template types where we need some (but not
  // all) of the template args as well.
}

void TestAutocast() {
  // We need full type of is2 because the declarer of Fn didn't
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  Fn(1, 2, 3, 4, 5);

  // We need full type of is2 because the declarer of Fn didn't
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplFn(6, 7, 8, 9, 10);
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

tests/iwyu_stricter_than_cpp.cc should add these lines:
#include "tests/iwyu_stricter_than_cpp-i2.h"
struct DirectStruct1;
struct DirectStruct2;
struct IndirectStruct1;
struct IndirectStructForwardDeclaredInD1;
template <typename T> struct TplDirectStruct1;
template <typename T> struct TplDirectStruct2;
template <typename T> struct TplIndirectStruct1;
template <typename T> struct TplIndirectStructForwardDeclaredInD1;

tests/iwyu_stricter_than_cpp.cc should remove these lines:
- #include "tests/iwyu_stricter_than_cpp-autocast2.h"  // lines XX-XX
- #include "tests/iwyu_stricter_than_cpp-d2.h"  // lines XX-XX

The full include-list for tests/iwyu_stricter_than_cpp.cc:
#include "tests/iwyu_stricter_than_cpp-autocast.h"  // for Fn, TplFn
#include "tests/iwyu_stricter_than_cpp-fnreturn.h"  // for DoesEverythingRightFn, DoesNotForwardDeclareAndIncludesFn, DoesNotForwardDeclareFn, DoesNotForwardDeclareProperlyFn, IncludesFn, TplDoesEverythingRightAgainFn, TplDoesEverythingRightFn, TplDoesNotForwardDeclareAndIncludesFn, TplDoesNotForwardDeclareFn, TplDoesNotForwardDeclareProperlyFn, TplIncludesFn
#include "tests/iwyu_stricter_than_cpp-i2.h"  // for IndirectStruct2, TplIndirectStruct2
#include "tests/iwyu_stricter_than_cpp-typedefs.h"  // for DoesEverythingRight, DoesNotForwardDeclare, DoesNotForwardDeclareAndIncludes, DoesNotForwardDeclareProperly, Includes, TplDoesEverythingRight, TplDoesEverythingRightAgain, TplDoesNotForwardDeclare, TplDoesNotForwardDeclareAndIncludes, TplDoesNotForwardDeclareProperly, TplIncludes
struct DirectStruct1;
struct DirectStruct2;
struct IndirectStruct1;
struct IndirectStructForwardDeclaredInD1;
template <typename T> struct TplDirectStruct1;
template <typename T> struct TplDirectStruct2;
template <typename T> struct TplIndirectStruct1;
template <typename T> struct TplIndirectStructForwardDeclaredInD1;

***** IWYU_SUMMARY */
