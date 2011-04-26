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
// We include this so the second declaration of TwiceDeclaredFunction
// is visible in the translation unit (but not by -d2.h)
#include "tests/iwyu_stricter_than_cpp-autocast2.h"
#include "tests/iwyu_stricter_than_cpp-d2.h"

typedef DoesEverythingRight DoubleTypedef;

// If the typedef in -typedefs.h requires the full type, then users of
// that typedef (here) do not.  Otherwise, they do.
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

// Now test autocast
void TestAutocast() {
  // We need full type of is2 because the declarer of Fn didn't
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  Fn(1, 2, 3, 4, 5);

  // We need full type of is2 because the declarer of Fn didn't
  // IWYU: IndirectStruct2 is...*iwyu_stricter_than_cpp-i2.h
  TplFn(6, 7, 8, 9, 10);
}


/**** IWYU_SUMMARY

tests/iwyu_stricter_than_cpp.cc should add these lines:
#include "tests/iwyu_stricter_than_cpp-i2.h"

tests/iwyu_stricter_than_cpp.cc should remove these lines:
- #include "tests/iwyu_stricter_than_cpp-autocast2.h"  // lines XX-XX
- #include "tests/iwyu_stricter_than_cpp-d2.h"  // lines XX-XX

The full include-list for tests/iwyu_stricter_than_cpp.cc:
#include "tests/iwyu_stricter_than_cpp-autocast.h"  // for Fn, TplFn
#include "tests/iwyu_stricter_than_cpp-i2.h"  // for IndirectStruct2, TplIndirectStruct2
#include "tests/iwyu_stricter_than_cpp-typedefs.h"  // for DoesEverythingRight, DoesNotForwardDeclare, DoesNotForwardDeclareAndIncludes, DoesNotForwardDeclareProperly, Includes, TplDoesEverythingRight, TplDoesEverythingRightAgain, TplDoesNotForwardDeclare, TplDoesNotForwardDeclareAndIncludes, TplDoesNotForwardDeclareProperly, TplIncludes

***** IWYU_SUMMARY */
