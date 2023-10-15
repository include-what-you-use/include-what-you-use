//===--- binary_type_trait.cc - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "tests/cxx/binary_type_trait-d1.h"

int main() {
    // IWYU: BinaryTypeTraitBase is...*binary_type_trait-i1.h
    // IWYU: BinaryTypeTraitBase needs a declaration
    // IWYU: BinaryTypeTraitDerived is...*binary_type_trait-i2.h
    // IWYU: BinaryTypeTraitDerived needs a declaration
    static_assert(__is_convertible_to(BinaryTypeTraitDerived*, BinaryTypeTraitBase*),
        "Derived should be convertible to the Base class");

    // IWYU: BinaryTypeTraitBase needs a declaration
    // IWYU: BinaryTypeTraitDerived needs a declaration
    static_assert(!__is_convertible_to(BinaryTypeTraitDerived**, BinaryTypeTraitBase**),
        "Indirect pointers shouldn't be convertible");

    // IWYU: BinaryTypeTraitBase is...*binary_type_trait-i1.h
    // IWYU: BinaryTypeTraitBase needs a declaration
    // IWYU: BinaryTypeTraitDerived is...*tests/cxx/binary_type_trait-i2.h
    // IWYU: BinaryTypeTraitDerived needs a declaration
    static_assert(__is_convertible_to(BinaryTypeTraitDerived&, BinaryTypeTraitBase&),
        "Derived should be convertible to the Base class");
}

/**** IWYU_SUMMARY

tests/cxx/binary_type_trait.cc should add these lines:
#include "tests/cxx/binary_type_trait-i1.h"
#include "tests/cxx/binary_type_trait-i2.h"

tests/cxx/binary_type_trait.cc should remove these lines:
- #include "tests/cxx/binary_type_trait-d1.h"  // lines XX-XX

The full include-list for tests/cxx/binary_type_trait.cc:
#include "tests/cxx/binary_type_trait-i1.h"  // for BinaryTypeTraitBase
#include "tests/cxx/binary_type_trait-i2.h"  // for BinaryTypeTraitDerived

***** IWYU_SUMMARY */
